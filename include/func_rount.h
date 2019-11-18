#pragma once

#include <mysql_func.h>
#include <common.h>

#define DB_MSG_HANDLE_FUN_PAR USER_ID uid, google::protobuf::Message *msg

DEF_MSG_HANDLE_FUN(func_route_t, DB)

class func_route_t : public func_route_base_t
{
public:
    int do_handle_dispatcher(const void *data, uint32_t len, xr::tcp_peer_t *peer);

    void init();

    void encode_msg2sendbuf(google::protobuf::Message *msg);

    uint32_t server_id;   //S3001=>1
    uint32_t platform_id; //S3001=>3

protected:
    void init_msg_map();

protected:
//bind cmd and func
#undef BIND_PROTO_CMD
#undef BIND_PROTO_CMD_NO_CB

#define BIND_PROTO_CMD(cmd, fun_name, proto_name) \
    int fun_name(DB_MSG_HANDLE_FUN_PAR)
#define BIND_PROTO_CMD_NO_CB(cmd, fun_name, proto_name) \
    int fun_name(DB_MSG_HANDLE_FUN_PAR)

    //#include <db_cmd.h>

#undef BIND_PROTO_CMD
#undef BIND_PROTO_CMD_NO_CB

public:
    func_route_t(xr_mysql::mysql_if_t *db);

    DB_MSG_MAP_T m_msgMap;
};

extern std::string g_db_name_pre;
const char *g_gen_db_name(const char *db_name);
extern func_route_t *g_route_func;
