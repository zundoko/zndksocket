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
 * @date     2017-01-09
 * @author   zundoko
 */

#ifndef    LAN_MSG_H
#define    LAN_MSG_H

/* definitions */
#define  PORT_TCP                 50001

/* LAN msg ID */
#define  LANMSG_ID_TEST         0x99990
#define  LANMSG_ID_GET_VERSION  0x10000

#define  LANMSG_TYPE_UNKNOWN         -1
#define  LANMSG_TYPE_TEST             0
#define  LANMSG_TYPE_GET_VERSION      1
#define  N_LANMSG_TYPES               2

/* LAN msg size (unit: [B]) */
#define  LEN_LANMSG_HEADER            8 /* header part: msgid + msgsz */
#define  LEN_LANMSG_TEST           2048
#define  LEN_LANMSG_GET_VERSION       0

#define    N_LANMSG_TEST          (LEN_LANMSG_TEST   / sizeof(uint32_t))
#define    N_LANMSG_RET_TEST          0
#define    N_LANMSG_RET_VERSION       1

#define  LANMSG_HEADER_ID_MSGID       0
#define  LANMSG_HEADER_ID_MSGSZ       1
#define    N_LANMSG_HEADER_IDS        2


/* extern declarations */
extern   int                read_lanmsg(int fd, int len, void *rbuf);
extern   int            get_lanmsg_type(uint32_t msgid);

#endif  /* LAN_MSG_H */

/* end */

