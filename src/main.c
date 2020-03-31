#include <signal.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "ops_net.h"
#include "main.h"

struct uds_session_t {
    uint8_t is_used;
    uint8_t magic;
    uint16_t index;
    struct sockaddr_un cli_addr;
    socklen_t cli_addr_len;
} __attribute__ ((packed));

static int socket_fd = -1;
static struct uds_session_t session[MAX_CLIENT_LOG];

int main(int argc, char** argv)
{
    struct ops_net_t *net = get_net_instance();
    //struct ops_shell_t *shell = get_shell_instance();
    struct msg_t req;
    struct msg_t res;
    //int rc = 0;
    //int wc = 0;
    int i = 0;
    uint32_t magic = 0;
    struct uds_session_t *uds = NULL;

    for(i=0;i<MAX_CLIENT_LOG;i++) {
        uds = &session[i];
	memset(uds, 0, sizeof(struct uds_session_t));
	uds->is_used = 0;
	uds->cli_addr_len = sizeof(struct sockaddr_un);
	uds->index = i;
    }
    uds = NULL;

    socket_fd = net->uds_server_create(SOCKET_PATH_LOG);
    if(socket_fd < 0) {
        return 1;
    }

    while(1) {
        for(i=0;i<MAX_CLIENT_LOG;i++) {
            uds = &session[i];
            if(uds->is_used) {
                continue;
            } else {
                uds->is_used = 1;
                magic += 1;
                memset(&uds->cli_addr, 0, sizeof(struct sockaddr_un));
		memset(&req, 0, sizeof(struct msg_t));
		memset(&res, 0, sizeof(struct msg_t));
                uds->magic = magic;
                net->uds_server_recv(socket_fd, &req, &uds->cli_addr, &uds->cli_addr_len);
                //shell_status = shell->process(req, res);
		res.fn = req.fn;
		res.cmd = req.cmd;
		sprintf(res.data, "[%03ld]: %s", magic, res.data);
		res.data_size = strlen(res.data);
                net->uds_server_send(socket_fd, &res, &uds->cli_addr, uds->cli_addr_len);
                uds->is_used = 0;
            }
        }
    }

    return 0;
}

