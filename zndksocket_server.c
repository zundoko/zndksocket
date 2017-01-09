/**
 * @file     zndksocket_server.c
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
#include <unistd.h>             /* close()       */
#include <errno.h>              /* errno         */

#include <sys/types.h>          /* socket        */
#include <sys/socket.h>         /* socket        */
#include <netinet/in.h>         /* socket        */
#include <arpa/inet.h>          /* inet_ntoar()  */

#include "lan_msg.h"            /* LAN msg           */
#include "zndksocket.h"         /* zndksocket header */

/**
 * _parse_lan_msg()
 * @brief    LAN msg parserm responder
 *
 * @param    [in]   sk_dst      int ::= socket fd for DST
 * @param    [in]   msgid  uint32_t ::= LAN msg ID
 * @param    [in]   msgsz  uint32_t ::= LAN msg size (unit: [B])
 * @param    [in]  *data   uint32_t ::= data part of LAN msg
 * @return          stat            ::= process status
 */
static int
_parse_lan_msg(int sk_dst, uint32_t msgid, uint32_t msgsz, uint32_t *data)
{
    int       stat = 0;
    int       type;
    int       ndata;
    uint32_t  wbuf[10];
    uint32_t *oheader = wbuf + 0;
    uint32_t *odata   = wbuf + 2;
    int       num;

    type = get_lanmsg_type(msgid);
    switch (type) {
    case LANMSG_TYPE_TEST       :
        printf("LANMSG_ID_TEST:[L%4d]\n", __LINE__);
        ndata      =  N_LANMSG_RET_TEST;
        msgid      =  LANMSG_ID_TEST | 0x01;
        msgsz      = (ndata * sizeof(uint32_t));
        oheader[0] =  htonl( msgid );
        oheader[1] =  htonl( msgsz );
        printf(" -> send msg to client\n");        
        num        =  send(sk_dst, wbuf, (LEN_LANMSG_HEADER + msgsz), 0);
        if (num    <  0) {
            perror(" lanmsg: send(): failure:");
        }
        break;
    case LANMSG_TYPE_GET_VERSION:
        printf("LANMSG_ID_GET_VERSION:[L%4d]\n", __LINE__);
        ndata      =  N_LANMSG_RET_VERSION;
        msgid      =  LANMSG_ID_GET_VERSION | 0x01;
        msgsz      = (ndata * sizeof(uint32_t));
        oheader[0] =  htonl( msgid      );
        oheader[1] =  htonl( msgsz      );
        odata[0]   =  htonl( 0x00001701 );
        printf(" -> send msg to client\n");
        num        =  send(sk_dst, wbuf, (LEN_LANMSG_HEADER + msgsz), 0);
        if (num    <  0) {
            perror(" lanmsg: send(): failure:");
        }
        break;
    default:
        printf(" %s():[L%4d]: unknown LAN msg ID = %05X\n", __func__, __LINE__, msgid);
        break;
    }

    return  stat;
}

/**
 * main()
 * @brief    main routine for mq rx
 *
 * @param    [in]   argc        int ::= # of args
 * @param    [in]  *argv[]     char ::= entity of args
 * @return          stat            ::= process termination (EXIT_)
 */
int
main(int argc, char *argv[])
{
    int                 stat   =  EXIT_SUCCESS;

    int                 sk_src;
    struct sockaddr_in  addr_src;

    int                 sk_dst;
    struct sockaddr_in  addr_dst;
    socklen_t           len_addr = sizeof(struct sockaddr_in);

    uint32_t            header[N_LANMSG_HEADER_IDS];
    int                 cnt = 0;

    while (1) {
        sk_src                   = socket(AF_INET, SOCK_STREAM, 0);
        if (sk_src < 0) {
            perror(" sock(): failure:");
            exit(1);
        }

        memset(&addr_src, 0, len_addr);
        addr_src.sin_family      = AF_INET;
        addr_src.sin_port        = htons(PORT_TCP);
        addr_src.sin_addr.s_addr = INADDR_ANY;
        bind(sk_src, (struct sockaddr *)&addr_src, len_addr);
        listen(sk_src, 1);

        printf("=== zndksocket_server start ===\n");

        sk_dst = accept(sk_src, (struct sockaddr *)&addr_dst, &len_addr);
        printf("=== zndksocket_server accepted: ip=%s ===\n", inet_ntoa(addr_dst.sin_addr));
        close(sk_src);

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
        while (1) {
            stat = read_lanmsg(sk_dst, 8, header);
            if (stat  < 0) {
                goto  err_main;
            } else {
                uint32_t  msgid = ntohl( header[LANMSG_HEADER_ID_MSGID] );
                uint32_t  msgsz = ntohl( header[LANMSG_HEADER_ID_MSGSZ] );

                printf(" server: fd=%d: msgid=%08X, msgsz=%4d (cnt=%d)\n", sk_dst, msgid, msgsz, cnt);
                {
                    uint32_t *data;

                    data = malloc(msgsz);
                    {
                        stat = read_lanmsg(sk_dst, msgsz, data);
                        if (stat < 0) {
                            free(data);
                            goto  err_main;
                        }
                        stat = _parse_lan_msg(sk_dst, msgid, msgsz, data);
                        if (stat < 0) {
                            free(data);
                            goto  err_main;
                        }
                    }
                    free(data);
                }
            }
            cnt++;
        }
    }

 err_main:
    close(sk_dst);              /* terminates the TCP session */

    printf("=== zndksocket_server end   ===\n");

    return  stat;
}

/* end */
