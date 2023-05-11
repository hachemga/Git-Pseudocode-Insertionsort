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

void print_packet(packet *pPacket);

/**
 * @brief Forward a packet to a peer.
 *
 * @param peer The peer to forward the request to
 * @param pack The packet to forward
 * @return int The status of the sending procedure
 */
int forward(peer *p, packet *pack) {
    /* TODO IMPLEMENT */
    if (peer_connect(p) < 0) {
        return 0;
    }
    size_t size_BUFFER;
    unsigned char *to_send = packet_serialize(pack, &size_BUFFER);
    if (sendall(p->socket, to_send, size_BUFFER) < 0) {
        return 0;
    }
    peer_disconnect(p);
    return CB_REMOVE_CLIENT;
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
    /* TODO IMPLEMENT */
    if (peer_connect(n) < 0) {
        return 0;
    }
    size_t size_BUFFER;
    unsigned char *to_send = packet_serialize(p, &size_BUFFER);
    //printf("socket: %d",n->socket);
    sendall(n->socket, to_send, size_BUFFER) ;
    //puts("sent number one\n");
    //fflush(stdout);
    size_t size_BUFFER2;
    printf("socket: %d",n->socket);
    unsigned char *to_recv = recvall(n->socket, &size_BUFFER2) ;
    //puts("Received from successor");
    peer_disconnect(n);
    if (sendall(csocket, to_recv, size_BUFFER2) < 0) {
        return 0;
    }
    return CB_REMOVE_CLIENT;
}

/**
 * @brief Lookup the peer responsible for a hash_id.
 *
 * @param hash_id The hash to lookup
 * @return int The callback status
 */
int lookup_peer(uint16_t hash_id) {
    /* TODO IMPLEMENT */
    // nasna3 packet kima f pdf hasb lookup n3abi flags w nabathou l succ bel fonctopn forward
    packet *new_packet;
    new_packet = packet_new();
    new_packet->flags = PKT_FLAG_CTRL | PKT_FLAG_LKUP;
    new_packet->hash_id = hash_id;
    new_packet->node_id = self->node_id;
    new_packet->node_port = self->port;
    new_packet->node_ip = peer_get_ip(self);
    forward(succ, new_packet);
    return CB_OK;
}

/**
 * @brief Handle a client request we are resonspible for.
 *
 * @param srv The server
 * @param c The client
 * @param p The packet
 * @return int The callback status
 */
int handle_own_request(server *srv, client *c, packet *p) {
    /* TODO IMPLEMENT */
    htable *r;
    switch (p->flags) {
        case PKT_FLAG_GET:
            r = htable_get(ht, p->key, p->key_len);
            if (r == NULL) {
                packet *new_packet;
                new_packet = packet_new();
                new_packet->key = p->key;
                new_packet->key_len = p->key_len;
                new_packet->flags = PKT_FLAG_ACK | PKT_FLAG_GET;
                size_t size_BUFFER;
                unsigned char *to_send = packet_serialize(new_packet, &size_BUFFER);
                if (sendall(c->socket, to_send, size_BUFFER) < 0) {
                    return 0;
                }
                free(new_packet);
            } else {
                packet *new_packet;
                new_packet = packet_new();
                new_packet->key = r->key;
                new_packet->key_len = r->key_len;
                new_packet->value = r->value;
                new_packet->value_len = r->value_len;
                new_packet->flags = PKT_FLAG_ACK | PKT_FLAG_GET;
                size_t size_BUFFER;
                unsigned char *to_send = packet_serialize(new_packet, &size_BUFFER);
                if (sendall(c->socket, to_send, size_BUFFER) < 0) {
                    return 0;
                }
                free(new_packet);
            }
            break;
        case PKT_FLAG_SET:
            htable_set(ht, p->key, p->key_len, p->value, p->value_len);
            //printf("htable-setted\n");
            packet *new_packet2;
            new_packet2 = packet_new();
            //printf("packet-newed\n");
            new_packet2->flags = PKT_FLAG_ACK | PKT_FLAG_SET;
            //printf("packet-flags-setted\n");
            size_t size_BUFFER2;
            unsigned char *to_send2 = packet_serialize(new_packet2, &size_BUFFER2);
            //printf("packet-serialized\n");
            //printf("socket: %d\n", c->socket);
            sendall(c->socket, to_send2, size_BUFFER2);
            //printf("packet-sent\n");
            packet_free(new_packet2);
            break;
        case PKT_FLAG_DEL:
            htable_delete(ht, p->key, p->key_len);
            packet *new_packet3;
            new_packet3 = packet_new();
            new_packet3->flags = PKT_FLAG_ACK | PKT_FLAG_DEL;
            size_t size_BUFFER3;
            unsigned char *to_send3 = packet_serialize(new_packet3, &size_BUFFER3);
            if (sendall(c->socket, to_send3, size_BUFFER3) < 0) {
                return 0;
            }
            packet_free(new_packet3);
            break;
    }
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
    /* TODO IMPLEMENT */
    //peerfrompacket t5arajli peer mel packet nabathlou bel forward l reply packet bel infos mta3 peer n w flags mtaa3 reply w control

    peer *newpeer= peer_from_packet(p);
    packet *new_packet;
    new_packet = packet_new();
    new_packet->node_id = n->node_id;
    new_packet->node_ip = peer_get_ip(n);
    new_packet->node_port = n->port;
    new_packet->flags = PKT_FLAG_CTRL | PKT_FLAG_RPLY;
    new_packet->hash_id = p->hash_id;
    forward(newpeer, new_packet);
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
    }
    return CB_REMOVE_CLIENT;
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

/**
 * @brief Main entry for a peer of the chord ring.
 *
 * Requires 9 arguments:
 * 1. Id
 * 2. Hostname
 * 3. Port
 * 4. Id of the predecessor
 * 5. Hostname of the predecessor
 * 6. Port of the predecessor
 * 7. Id of the successor
 * 8. Hostname of the successor
 * 9. Port of the successor
 *
 * @param argc The number of arguments
 * @param argv The arguments
 * @return int The exit code
 */
int main(int argc, char **argv) {

    if (argc < 10) {
        fprintf(stderr, "Not enough args! I need ID IP PORT ID_P IP_P PORT_P "
                        "ID_S IP_S PORT_S\n");
    }

    // Read arguments for self
    uint16_t idSelf = strtoul(argv[1], NULL, 10);
    char *hostSelf = argv[2];
    char *portSelf = argv[3];

    // Read arguments for predecessor
    uint16_t idPred = strtoul(argv[4], NULL, 10);
    char *hostPred = argv[5];
    char *portPred = argv[6];

    // Read arguments for successor
    uint16_t idSucc = strtoul(argv[7], NULL, 10);
    char *hostSucc = argv[8];
    char *portSucc = argv[9];

    // Initialize all chord peers
    self = peer_init(
        idSelf, hostSelf,
        portSelf); //  Not really necessary but convenient to store us as a peer
    pred = peer_init(idPred, hostPred, portPred); //

    succ = peer_init(idSucc, hostSucc, portSucc);

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
}
