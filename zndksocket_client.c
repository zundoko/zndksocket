/**
 * @file     zndksocket_client.c
 * @brief    socket programming test
 *
 * @note
 *
 * @date     2017-01-09
 * @author   zundoko
 */

#include <stdio.h>              /* printf()      */
#include <stdlib.h>             /* EXIT_SUCCESS  */
#include <string.h>             /* memset()      */
#include <unistd.h>             /* sleep()       */

#include <sys/types.h>          /* socket        */
#include <sys/socket.h>         /* socket        */
#include <netinet/in.h>         /* socket        */
#include <arpa/inet.h>          /* inet_addr()   */

#include "lan_msg.h"            /* LAN msg           */
#include "zndksocket.h"         /* zndksocket header */

/**
 * main()
 * @brief    main routine for mq tx
 *
 * @param    [in]   argc        int ::= # of args
 * @param    [in]  *argv[]     char ::= entity of args
 * @return          stat            ::= process termination (EXIT_)
 */
int
main(int argc, char *argv[])
{
    int                 stat   = EXIT_SUCCESS;
    int                 skfd;
    struct sockaddr_in  server;

    skfd = socket(AF_INET, SOCK_STREAM, 0);

    /* prepares for a server */
    memset(&server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT_TCP);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* connects to the server */
    connect(skfd, (struct sockaddr *)&server, sizeof(server));

    printf("=== zndksocket_client start ===\n");

    /* sends a message to the server */
    {
        /*
         * LAN msg format:
         *      word
         *      idx   31            16 15             0
         *     ------+----------------+----------------+ -+-
         *       0   |              msgid              |  | header part
         *       1   |              msgsz              |  |
         *     ------+---------------------------------+ -+-
         *       2   |              msg 0              |  | data   part
         *       :   :               :                 :  :
         *    2+(N-1)|              msg N-1            |  |
         *     ------+---------------------------------+ -+-
         *             byte-order: big endian
         * 
         *       where:
         *         msgid: LAN msg ID
         *         msgsz: length of data part (unit: [B]), multiple of four
         *         msg  : LAN msg
         *         N    : msgsz / sizeof(uint32_t), # of 32-bit words
         */
        uint32_t  buf[2 + N_LANMSG_TEST];
        uint32_t *header     = buf + 0;
        uint32_t *data       = buf + 2;
        int       idx;
        int       num;
        int       cnt = 0;

        /* LAN msg test */
        header[0] = htonl( LANMSG_ID_TEST  );
        header[1] = htonl( LEN_LANMSG_TEST );
        for (idx = 0; idx < N_LANMSG_TEST; idx++) {
            data[idx] = htonl( idx );
        }

        while (cnt < 2) {
            /* LAN msg test */
            header[0] = htonl( LANMSG_ID_TEST  );
            header[1] = htonl( LEN_LANMSG_TEST );
            for (idx = 0; idx < N_LANMSG_TEST; idx++) {
                data[idx] = htonl( idx );
            }

            printf(" <- send msg to server (cnt=%d, %ld)\n", cnt, sizeof(buf));
            printf(" <- send msg to server: fd=%d: msgid=%08X, msgsz=%4d\n", skfd, ntohl(header[0]), ntohl(header[1]));
            num = send(skfd, buf, sizeof(buf), 0);
            if (num < 0) {
                perror(" client: send(): failure:");
                goto  err_main;
            }
            sleep(1);

            stat = read_lanmsg(skfd, 8, header);
            if (stat  < 0) {
                goto  err_main;
            } else {
                uint32_t  msgid = ntohl( header[0] );
                uint32_t  msgsz = ntohl( header[1] );

                printf(" client: fd=%d: msgid=%08X, msgsz=%4d\n", skfd, msgid, msgsz);
                if (msgsz > 0) {
                    uint32_t *data;
                    int       idx;
                    data = malloc(msgsz);
                    {
                        stat = read_lanmsg(skfd, LEN_LANMSG_HEADER, data);
                        if (stat < 0) {
                            goto  err_main;
                        }
                        for (idx = 0; idx < (msgsz / sizeof(uint32_t)); idx++) {
                            printf(" client: %2d, %4d: %08d\n", cnt, idx, ntohl( data[idx]) );
                        }
                    }
                    free(data);
                }
            }
            cnt++;
        }

        /* LAN msg: get version */
        header[0] = htonl( LANMSG_ID_GET_VERSION  );
        header[1] = htonl( 0                      );

        printf(" <- send msg to server\n");
        num = send(skfd, buf, LEN_LANMSG_HEADER, 0);
        if (num < 0) {
            perror(" client: send(): failure:");
            goto  err_main;
        }

        stat = read_lanmsg(skfd, 8, header);
        if (stat  < 0) {
            goto  err_main;
        } else {
            uint32_t  msgid = ntohl( ((uint32_t *)header)[0] );
            uint32_t  msgsz = ntohl( ((uint32_t *)header)[1] );

            printf(" client: fd=%d: msgid=%08X, msgsz=%4d\n", skfd, msgid, msgsz);
            if (msgsz > 0) {
                uint32_t *data;
                int       idx;
                data = malloc(msgsz);
                {
                    stat = read_lanmsg(skfd, msgsz, data);
                    if (stat < 0) {
                        goto  err_main;
                    }
                    for (idx = 0; idx < (msgsz / sizeof(uint32_t)); idx++) {
                        printf(" client: %2d, %4d: %08X\n", cnt, idx, ntohl( data[idx]) );
                    }
                }
                free(data);
            }
        }
    }

 err_main:
    close(skfd);

    printf("=== zndksocket_client end   ===\n");

    return  stat;
}

/* end */
