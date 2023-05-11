#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hash_table.h"
#include "neighbour.h"
#include "packet.h"
#include "requests.h"
#include "server.h"
#include "util.h"

// actual underlying hash table
htable **ht = NULL;
rtable **rt = NULL;

// chord peers
peer *self = NULL;
peer *pred = NULL;
peer *succ = NULL;

/**
 * @brief Forward a packet to a peer.
 *
 * @param peer The peer to forward the request to
 * @param pack The packet to forward
 * @return int The status of the sending procedure
 */
int forward(peer *p, packet *pack) {
    // check whether we can connect to the peer
    if (peer_connect(p) != 0) {
        fprintf(stderr, "Failed to connect to peer %s:%d\n", p->hostname,
                p->port);
        return -1;
    }

    size_t data_len;
    unsigned char *raw = packet_serialize(pack, &data_len);
    int status = sendall(p->socket, raw, data_len);
    free(raw);
    raw = NULL;

    peer_disconnect(p);
    return status;
}

/**
 * @brief Forward a request to the successor.
 *
 * @param srv The server
 * @param csocket The scokent of the client
 * @param p The packet to forward
 * @param n The peer to forward to
 * @return int The callback status
 */
int proxy_request(server *srv, int csocket, packet *p, peer *n) {
    // check whether we can connect to the peer
    if (peer_connect(n) != 0) {
        fprintf(stderr,
                "Could not connect to peer %s:%d to proxy request for client!",
                n->hostname, n->port);
        return CB_REMOVE_CLIENT;
    }

    size_t data_len;
    unsigned char *raw = packet_serialize(p, &data_len);
    sendall(n->socket, raw, data_len);
    free(raw);
    raw = NULL;
    size_t rsp_len = 0;
    unsigned char *rsp = recvall(n->socket, &rsp_len);
    // Just pipe everything through unfiltered. Yolo!
    sendall(csocket, rsp, rsp_len);
    free(rsp);

    return CB_REMOVE_CLIENT;
}


/**
 * @brief Lookup the peer responsible for a hash_id.
 *
 * @param hash_id The hash to lookup
 * @return int The callback status
 */
int lookup_peer(uint16_t hash_id) {
    // We could see whether or not we need to repeat the lookup

    // build a new packet for the lookup
    packet *lkp = packet_new();
    lkp->flags = PKT_FLAG_CTRL | PKT_FLAG_LKUP;
    lkp->hash_id = hash_id;
    lkp->node_id = self->node_id;
    lkp->node_port = self->port;

    lkp->node_ip = peer_get_ip(self);

    forward(succ, lkp);
    return 0;
}

/**
 * @brief Handle a client request we are resonspible for.
 *
 * @param c The client
 * @param p The packet
 * @return int The callback status
 */
int handle_own_request(server* srv, client *c, packet *p) {
    // build a new packet for the request
    packet *rsp = packet_new();

    if (p->flags & PKT_FLAG_GET) {
        // this is a GET request
        htable *entry = htable_get(ht, p->key, p->key_len);
        if (entry != NULL) {
            rsp->flags = PKT_FLAG_GET | PKT_FLAG_ACK;

            rsp->key = (unsigned char *)malloc(entry->key_len);
            rsp->key_len = entry->key_len;
            memcpy(rsp->key, entry->key, entry->key_len);

            rsp->value = (unsigned char *)malloc(entry->value_len);
            rsp->value_len = entry->value_len;
            memcpy(rsp->value, entry->value, entry->value_len);
        } else {
            rsp->flags = PKT_FLAG_GET;
            rsp->key = (unsigned char *)malloc(p->key_len);
            rsp->key_len = p->key_len;
            memcpy(rsp->key, p->key, p->key_len);
        }
    } else if (p->flags & PKT_FLAG_SET) {
        // this is a SET request
        rsp->flags = PKT_FLAG_SET | PKT_FLAG_ACK;
        htable_set(ht, p->key, p->key_len, p->value, p->value_len);
    } else if (p->flags & PKT_FLAG_DEL) {
        // this is a DELETE request
        int status = htable_delete(ht, p->key, p->key_len);

        if (status == 0) {
            rsp->flags = PKT_FLAG_DEL | PKT_FLAG_ACK;
        } else {
            rsp->flags = PKT_FLAG_DEL;
        }
    } else {
        // send some default data
        rsp->flags = p->flags | PKT_FLAG_ACK;
        rsp->key = (unsigned char *)strdup("Rick Astley");
        rsp->key_len = strlen((char *)rsp->key);
        rsp->value = (unsigned char *)strdup("Never Gonna Give You Up!\n");
        rsp->value_len = strlen((char *)rsp->value);
    }

    size_t data_len;
    unsigned char *raw = packet_serialize(rsp, &data_len);
    free(rsp);
    sendall(c->socket, raw, data_len);
    free(raw);
    raw = NULL;

    return CB_REMOVE_CLIENT;
}

/**
 * @brief Answer a lookup request from a peer.
 *
 * @param p The packet
 * @param n The peer
 * @return int The callback status
 */
int answer_lookup(packet *p, peer *n) {
    peer *questioner = peer_from_packet(p);

    // check whether we can connect to the peer
    if (peer_connect(questioner) != 0) {
        fprintf(stderr, "Could not connect to questioner of lookup at %s:%d\n!",
                questioner->hostname, questioner->port);
        peer_free(questioner);
        return CB_REMOVE_CLIENT;
    }

    // build a new packet for the response
    packet *rsp = packet_new();
    rsp->flags = PKT_FLAG_CTRL | PKT_FLAG_RPLY;
    rsp->hash_id = p->hash_id;
    rsp->node_id = n->node_id;
    rsp->node_port = n->port;
    rsp->node_ip = peer_get_ip(n);

    size_t data_len;
    unsigned char *raw = packet_serialize(rsp, &data_len);
    free(rsp);
    sendall(questioner->socket, raw, data_len);
    free(raw);
    raw = NULL;
    peer_disconnect(questioner);
    peer_free(questioner);
    return CB_REMOVE_CLIENT;
}

/**
 * @brief Handle a key request request from a client.
 *
 * @param srv The server
 * @param c The client
 * @param p The packet
 * @return int The callback status
 */
int handle_packet_data(server *srv, client *c, packet *p) {
    // Hash the key of the <key, value> pair to use for the hash table
    uint16_t hash_id = pseudo_hash(p->key, p->key_len);
    fprintf(stderr, "Hash id: %d\n", hash_id);

    // Forward the packet to the correct peer
    if (peer_is_responsible(pred->node_id, self->node_id, hash_id)) {
        // We are responsible for this key
        fprintf(stderr, "We are responsible.\n");
        return handle_own_request(srv, c, p);
    } else if (peer_is_responsible(self->node_id, succ->node_id, hash_id)) {
        // Our successor is responsible for this key
        fprintf(stderr, "Successor's business.\n");
        return proxy_request(srv, c->socket, p, succ);
    } else {
        // We need to find the peer responsible for this key
        fprintf(stderr, "No idea! Just looking it up!.\n");
        add_request(rt, hash_id, c->socket, p);
        lookup_peer(hash_id);
        return CB_OK;
    }
}
void print_peer(peer *p) {
    if (p == NULL) {
        fprintf(stdout, "Peer: NULL");
        return;
    }
    fprintf(stdout, "Peer: %s:%d:%d\n", p->hostname, p->port, p->node_id);
}

/**
 * @brief Handle a control packet from another peer.
 * Lookup vs. Proxy Reply
 *
 * @param srv The server
 * @param c The client
 * @param p The packet
 * @return int The callback status
 */
int handle_packet_ctrl(server *srv, client *c, packet *p) {

    fprintf(stderr, "Handling control packet...\n");

    if (p->flags & PKT_FLAG_LKUP) {
        // we received a lookup request
        if (peer_is_responsible(pred->node_id, self->node_id, p->hash_id)) {
            // Our business
            fprintf(stderr, "Lol! This should not happen!\n");
            return answer_lookup(p, self);
        } else if (peer_is_responsible(self->node_id, succ->node_id,
                                       p->hash_id)) {
            return answer_lookup(p, succ);
        } else {
            // Great! Somebody else's job!
            forward(succ, p);
        }
    } else if (p->flags & PKT_FLAG_RPLY) {
        // Look for open requests and proxy them
        peer *n = peer_from_packet(p);
        for (request *r = get_requests(rt, p->hash_id); r != NULL;
             r = r->next) {
            proxy_request(srv, r->socket, r->packet, n);
            server_close_socket(srv, r->socket);
        }
        clear_requests(rt, p->hash_id);
    } else {
        /**
         * TODO:
         * Extend handled control messages.
         * For the first task, this means that join-, stabilize-, and notify-messages should be understood.
         * For the second task, finger- and f-ack-messages need to be used as well.
         **/
        if (p->flags & PKT_FLAG_JOIN) {
            //cas mta3 fama knot wehed
            // Handle join request
            peer *n = peer_from_packet(p);
            n->node_id = p->node_id;
            printf("node mock id: %d",n->node_id);
            printf("node mock port: %d",n->port);
            //nzod id lel peer mel packet
            print_peer(succ);
            print_peer(pred);
            if (pred == NULL && succ == NULL) {
                pred = n;
                succ = n;
                packet *rsp = packet_new();
                rsp->flags = PKT_FLAG_NTFY | PKT_FLAG_CTRL;
                rsp->node_id = self->node_id;
                rsp->node_port = self->port;
                rsp->node_ip = peer_get_ip(self);
                /*peer_connect(n);
                size_t size_BUFFER;
                unsigned char *to_send = packet_serialize(rsp, &size_BUFFER);
                sendall(n->socket, to_send, size_BUFFER);
                packet_free(rsp);
                peer_disconnect(n);*/
                forward(n, rsp);
            }else if (peer_is_responsible(pred->node_id, self->node_id, n->node_id)){

                pred = n;
                packet *rsp = packet_new();
                rsp->flags = PKT_FLAG_NTFY | PKT_FLAG_CTRL;
                //rsp->flags = PKT_FLAG_JOIN | PKT_FLAG_ACK;
                rsp->node_id = self->node_id;
                rsp->node_port = self->port;
                rsp->node_ip = peer_get_ip(self);
                /*peer_connect(n);
                size_t size_BUFFER2;
                unsigned char *to_send = packet_serialize(rsp, &size_BUFFER2);
                sendall(n->socket, to_send, size_BUFFER2);
                packet_free(rsp);
                peer_disconnect(n);*/
                forward(n, rsp);
            } else {
                // forward the join request
                forward(succ, p);
            }
        } else if (p->flags & PKT_FLAG_STAB) {
            // Handle stabilize request
            //l chkoun bech nabath w chnoua bech nabath w kifeh bech nabath
            //nraja3 notify eli tab3ath f stabilize fiha les infos mta3 pred actuelmement
            //kifeh bech nabath: nzod id lel peer mel packet w nzod id lel pred w nzod id lel succ juste l teri5
            peer *n = peer_from_packet(p);
            n->node_id = p->node_id;
            if (pred == NULL) {
                pred = n;
            }
                packet *rsp = packet_new();
                rsp->flags = PKT_FLAG_NTFY | PKT_FLAG_CTRL;
                //rsp->flags = PKT_FLAG_JOIN | PKT_FLAG_ACK;
                rsp->node_id = pred->node_id;
                rsp->node_port = pred->port;
                rsp->node_ip = peer_get_ip(pred);
                size_t size_BUFFER4;
                unsigned char *to_send = packet_serialize(rsp, &size_BUFFER4);
                sendall(c->socket, to_send, size_BUFFER4);
                packet_free(rsp);

        } else if (p->flags & PKT_FLAG_NTFY) {
            if (succ == NULL || peer_is_responsible(self->node_id, succ->node_id, p->node_id)) {
                succ = peer_from_packet(p);
                succ->node_id = p->node_id;
            }

        } else if (p->flags & PKT_FLAG_FNGR) {
            int i;
            uint16_t start;
            peer *successor;
            peer *n = peer_from_packet(p);
            n->node_id = p->node_id;
            for (i = 0; i < 16; i++) {
                start = p->hash_id + (1 << i);
                successor = lookup_peer(start);
                n->ft[i] = successor;
            }
            packet *rsp = packet_new();
            rsp->flags = PKT_FLAG_FNGR | PKT_FLAG_FACK;
            size_t size_BUFFER5;
            unsigned char *to_send = packet_serialize(rsp, &size_BUFFER5);
            sendall(c->socket, to_send, size_BUFFER5);
            packet_free(rsp);
        }
    }
    return CB_REMOVE_CLIENT;
}

int connect_socket(char *hostname, char *port) {
    struct addrinfo *res;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname, port, &hints, &res);
    if (status != 0) {
        perror("getaddrinfo:");
        return -1;
    }

    struct addrinfo *p;
    int sock = -1;
    bool connected = false;

    char ipstr[INET6_ADDRSTRLEN];

    for (p = res; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) {
            continue;
        }

        get_ip_str(p->ai_addr, ipstr, INET6_ADDRSTRLEN);
        fprintf(stderr, "Attempting connection to %s\n", ipstr);

        status = connect(sock, p->ai_addr, p->ai_addrlen);
        if (status < 0) {
            perror("connect");
            close(sock);
            continue;
        }
        connected = true;
        break;
    }
    freeaddrinfo(res);

    if (!connected) {
        return -1;
    }

    fprintf(stderr, "Connected to %s.\n", ipstr);

    return sock;
}

void join(char* ip, char* port) {
    // Send join request to target
    packet *p = packet_new();
    p->flags = PKT_FLAG_CTRL | PKT_FLAG_JOIN;
    p->node_id = self->node_id;
    p->node_ip = peer_get_ip(self);
    p->node_port = self->port;
    int sock = connect_socket (ip, port);
    size_t size_BUFFER5;
    unsigned char *to_send = packet_serialize(p, &size_BUFFER5);
    sendall(sock, to_send, size_BUFFER5);
    packet_free(p);
}
/**
 * @brief Handle a received packet.
 * This can be a key request received from a client or a control packet from
 * another peer.
 *
 * @param srv The server instance
 * @param c The client instance
 * @param p The packet instance
 * @return int The callback status
 */
int handle_packet(server *srv, client *c, packet *p) {
    if (p->flags & PKT_FLAG_CTRL) {
        return handle_packet_ctrl(srv, c, p);
    } else {
        return handle_packet_data(srv, c, p);
    }
}
void stabilize(server *srv) {
    if (succ != NULL) {
        // Send stabilize message to successor
        packet *pack = packet_new();
        pack->flags = PKT_FLAG_CTRL | PKT_FLAG_STAB;
        pack->node_id = self->node_id;
        pack->node_ip = peer_get_ip(self);
        pack->node_port = self->port;
        peer_connect(succ);
        size_t size_BUFFER5;
        unsigned char *to_send = packet_serialize(pack, &size_BUFFER5);
        client *new_client = (client *) malloc(sizeof(client));
        new_client->socket = succ->socket;

        new_client->state = IDLE;
        new_client->header_buf = rb_new(PKT_HEADER_LEN);
        new_client->pkt_buf = NULL;
        new_client->pack = NULL;

        // Append to front
        new_client->next = srv->clients;
        srv->clients = new_client;
        srv->n_clients++;

        sendall(succ->socket, to_send, size_BUFFER5);

        packet_free(pack);
    }
}


/**
 * @brief Main entry for a peer of the chord ring.
 *
 * TODO:
 * Modify usage of peer. Accept:
 * 1. Own IP and port;
 * 2. Own ID (optional, zero if not passed);
 * 3. IP and port of Node in existing DHT. This is optional: If not passed, establish new DHT, otherwise join existing.
 *
 * @param argc The number of arguments
 * @param argv The arguments
 * @return int The exit code
 */
int main(int argc, char **argv) {
        if (argc < 3 || argc > 6){
        fprintf(stderr, "Usage: %s  [host] [port] [id] [join_host] [join_port]", argv[0]);
        }else if (argc == 3) {
        // Read arguments for self
        uint16_t idSelf = 0;
        char *hostSelf = argv[1];
        char *portSelf = argv[2];

        // Initialize all chord peers
        self = peer_init(idSelf, hostSelf, portSelf);

        // Set pred and succ to null
        pred = NULL;
        succ = NULL;

        // Initialize outer server for communication with clients
        server *srv = server_setup(portSelf);
        if (srv == NULL) {
            fprintf(stderr, "Server setup failed!\n");
            return -1;
        }

        // Initialize hash table
        ht = (htable **)malloc(sizeof(htable *));
        // Initialize request table
        rt = (rtable **)malloc(sizeof(rtable *));
        *ht = NULL;
        *rt = NULL;

        srv->packet_cb = handle_packet;
        server_run(srv);
        close(srv->socket);
    } else if (argc == 4){
        uint16_t idSelf = strtoul(argv[3], NULL, 10);
        char *hostSelf = argv[1];
        char *portSelf = argv[2];
        self = peer_init(
                idSelf, hostSelf,
                portSelf);
        // Initialize outer server for communication with clients
        server *srv = server_setup(portSelf);
        if (srv == NULL) {
            fprintf(stderr, "Server setup failed!\n");
            return -1;
        }

        // Initialize hash table
        ht = (htable **)malloc(sizeof(htable *));
        // Initialize request table
        rt = (rtable **)malloc(sizeof(rtable *));
        *ht = NULL;
        *rt = NULL;

        srv->packet_cb = handle_packet;
        server_run(srv);
        close(srv->socket);
    } else if (argc == 5){
        uint16_t idSelf = 0;
        char *hostSelf = argv[1];
        char *portSelf = argv[2];
        // Read arguments for peer in existing DHT
        char *hostPeer = argv[3];
        char *portPeer = argv[4];
        // Initialize all chord peers
        self = peer_init(idSelf, hostSelf, portSelf);
        // Send join request to peer in existing DHT
        join(argv[3], argv[4]);
        // Initialize outer server for communication with clients
        server *srv = server_setup(portSelf);
        if (srv == NULL) {
            fprintf(stderr, "Server setup failed!\n");
            return -1;
        }

        // Initialize hash table
        ht = (htable **)malloc(sizeof(htable *));
        // Initiale reuqest table
        rt = (rtable **)malloc(sizeof(rtable *));
        *ht = NULL;
        *rt = NULL;

        srv->packet_cb = handle_packet;
        server_run(srv);
        close(srv->socket);
    }else if (argc == 6){
        uint16_t idSelf = strtoul(argv[3], NULL, 10);
        char *hostSelf = argv[1];
        char *portSelf = argv[2];
        // Read arguments for peer in existing DHT
        char *hostPeer = argv[4];
        char *portPeer = argv[5];
        // Initialize all chord peers
        self = peer_init(idSelf, hostSelf, portSelf);
        // Send join request to peer in existing DHT
        join(argv[4], argv[5]);
        // Initialize outer server for communication with clients
        server *srv = server_setup(portSelf);
        if (srv == NULL) {
            fprintf(stderr, "Server setup failed!\n");
            return -1;
        }

        // Initialize hash table
        ht = (htable **)malloc(sizeof(htable *));
        // Initiale reuqest table
        rt = (rtable **)malloc(sizeof(rtable *));
        *ht = NULL;
        *rt = NULL;

        srv->packet_cb = handle_packet;
        server_run(srv);
        close(srv->socket);;
    }


}
