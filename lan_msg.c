/**
 * @file     lan_msg.h
 * @brief    LAN msg
 *
 * @note     LAN msg format:
 * @verbatim
 *               word
 *               idx   31            16 15             0
 *              ------+----------------+----------------+ -+-
 *                0   |              msgid              |  | header part
 *                1   |              msgsz              |  |
 *              ------+---------------------------------+ -+-
 *                2   |              msg 0              |  | data   part
 *                :   :               :                 :  :
 *             2+(N-1)|              msg N-1            |  |
 *              ------+---------------------------------+ -+-
 *                      byte-order: big endian
 *          
 *                where:
 *                  msgid: LAN msg ID
 *                  msgsz: length of data part (unit: [B]), multiple of four
 *                  msg  : LAN msg
 *                  N    : msgsz / sizeof(uint32_t), # of 32-bit words
 * @endverbatim
 *
 * @date     2017-01-07
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

#include "lan_msg.h"            /* own header    */

/**
 * read_lanmsg()
 * @brief    reads a LAN msg
 *
 * @param    [in]   fd          int ::= socket fd
 * @param    [in]   len         int ::= length to be read (unit: [B])
 * @param    [out] *rbuf       void ::= read buffer
 * @return          stat            ::= process status
 */
int
read_lanmsg(int fd, int len, void *rbuf)
{
    int   stat   = 0;
    int   nbytes;
    int   remain = len;

    while (remain > 0) {
//        printf(" %s():[L%4d]: len=%d, remain=%d\n", __func__, __LINE__, len, remain);
        nbytes = recv(fd, rbuf, remain, 0);
        if        (nbytes  < 0) {
            perror(" recv():");
            printf(" %s():[L%4d]: receive error: nbytes=%d\n", __func__, __LINE__, nbytes);
            stat = -1;
            break;
        } else if (nbytes == 0) {
            printf(" %s():[L%4d]: connection closed, nbytes=%d\n", __func__, __LINE__, nbytes);
            stat = -2;
            break;
        }
//        printf(" %s():[L%4d]: len=%d, remain=%d, nbytes=%d\n", __func__, __LINE__, len, remain, nbytes);
        rbuf   += nbytes;
        remain -= nbytes;
    }

    return  stat;
}

/**
 * get_lanmsg_type()
 * @brief    get LAN msg type from header
 *
 * @param    [in]   msgid  uint32_t ::= LAN msg ID
 * @return          type            ::= LAN msg type
 */
int
get_lanmsg_type(uint32_t msgid)
{
    int       type          = 0;

    switch (msgid) {
    case LANMSG_ID_TEST       : type = LANMSG_TYPE_TEST;        break;
    case LANMSG_ID_GET_VERSION: type = LANMSG_TYPE_GET_VERSION; break;
    default                   : type = LANMSG_TYPE_UNKNOWN;     break;
    }

    return  type;
}

/* end */
