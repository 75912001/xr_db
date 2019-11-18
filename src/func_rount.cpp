#include "func_rount.h"
#include <proto_header.h>
#include <xr_log.h>
#include <mysql_func.h>

std::string g_db_name_pre;

char tmp_db_name[1024];

const char *g_gen_db_name(const char *db_name)
{
	::memset(tmp_db_name, 0, sizeof(tmp_db_name));
	::sprintf(tmp_db_name, "%s_%s", g_db_name_pre.c_str(), db_name);
	return tmp_db_name;
}

int func_route_t::do_handle_dispatcher(const void *data, uint32_t len, xr::tcp_peer_t *peer)
{
	this->head.unpack(data);
	this->peer = peer;

	char *body = (char *)data + xr_server::proto_head_t::PROTO_HEAD_LEN;
	uint32_t body_len = len - xr_server::proto_head_t::PROTO_HEAD_LEN;

	TRACE_LOG("<====== recv len:%u, cmd:%#x, seq:%u, ret:%#x, uid:%" PRIu64 ", fd:%d",
			  this->head.len, this->head.cmd, this->head.seq,
			  this->head.ret, this->head.uid, this->peer->fd);

	auto iter = m_msgMap.find(this->head.cmd);
	if (iter == m_msgMap.end())
	{
		ERROR_LOG("no msg define for 0x%x", head.cmd);
		return 0;
	}

	try
	{
		iter->second.prototype->Clear();
		if (!iter->second.prototype->ParseFromArray(body, body_len))
		{
			ERROR_LOG("decode message 0x%x with %u bytes failed", head.cmd, body_len);
			return 0;
		}
		if (!iter->second.prototype->Utf8DebugString().empty())
		{
			TRACE_LOG("%s", iter->second.prototype->Utf8DebugString().c_str());
		}
	}
	catch (...)
	{
		ERROR_LOG("parse message failed 0x%x ", head.cmd);
		return 0;
	}

	int ret = (this->*iter->second.func)(head.uid, iter->second.prototype);

	if (0 != ret)
	{
		this->db->rollback();
	}

	//提交数据
	this->db->commit();

	if (0 != ret)
	{
		this->send_ret(this->head, ret);
	}
	else
	{
		if (iter->second.is_callback)
		{
			this->s2peer();
		}
	}

	return 0;
}

func_route_t::func_route_t(xr_mysql::mysql_if_t *db)
	: func_route_base_t(db)
{
	this->init();
}

void func_route_t::init()
{
	uint32_t platform_server_id = atoi(g_db_name_pre.substr(1).c_str());
	this->server_id = platform_server_id % 10000;
	this->platform_id = platform_server_id / 10000;

	this->init_msg_map();
}

void func_route_t::encode_msg2sendbuf(google::protobuf::Message *msg)
{
	static unsigned char *buf = this->data + xr_server::proto_head_t::PROTO_HEAD_LEN;
	static uint32_t len = sizeof(this->data) - xr_server::proto_head_t::PROTO_HEAD_LEN;

	uint32_t body_len = msg->ByteSize();
	if (len < body_len)
	{ //要发送的数据长度大于缓冲区长度
		ERROR_LOG("no buf for 0x%x", this->head.cmd);
		return;
	}

	msg->SerializeToArray(buf, len);
	TRACE_LOG("======> send msg body len:%u, %s ", body_len, msg->Utf8DebugString().c_str());

	this->data_len = xr_server::proto_head_t::PROTO_HEAD_LEN + body_len;

	xr_server::proto_head_t::pack((char *)this->data, xr_server::proto_head_t::PROTO_HEAD_LEN + body_len, this->head.cmd, this->head.uid, this->head.seq, this->head.ret);
}

void func_route_t::init_msg_map()
{
#undef BIND_PROTO_CMD
#undef BIND_PROTO_CMD_NO_CB

#define BIND_PROTO_CMD(cmd, fun_name, proto_name)                                              \
	{                                                                                          \
		if (m_msgMap.end() != m_msgMap.find(cmd))                                              \
		{                                                                                      \
			ALERT_LOG("cmd inster err![0x%x]", cmd);                                           \
			exit(0);                                                                           \
		}                                                                                      \
		else                                                                                   \
		{                                                                                      \
			m_msgMap[cmd] = MSG_HANDLE_DB_T(new db_msg::proto_name(), &func_route_t::fun_name); \
		}                                                                                      \
	}

#define BIND_PROTO_CMD_NO_CB(cmd, fun_name, proto_name)                                               \
	{                                                                                                 \
		if (m_msgMap.end() != m_msgMap.find(cmd))                                                     \
		{                                                                                             \
			ALERT_LOG("cmd inster err![0x%x]", cmd);                                                  \
			exit(0);                                                                                  \
		}                                                                                             \
		else                                                                                          \
		{                                                                                             \
			m_msgMap[cmd] = MSG_HANDLE_DB_T(new db_msg::proto_name(), &func_route_t::fun_name, false); \
		}                                                                                             \
	}

	//#include <db_cmd.h>

#undef BIND_PROTO_CMD
#undef BIND_PROTO_CMD_NO_CB

	//////////////////////////////////////////////////////////////////////////

#undef BIND_PROTO_CMD
#undef BIND_PROTO_CMD_NO_CB

#define BIND_PROTO_CMD(cmd, fun_name, proto_name)                                                     \
	{                                                                                                 \
		if (m_msgMap.end() != m_msgMap.find(cmd))                                                     \
		{                                                                                             \
			ALERT_LOG("cmd inster err![0x%x]", cmd);                                                  \
			exit(0);                                                                                  \
		}                                                                                             \
		else                                                                                          \
		{                                                                                             \
			m_msgMap[cmd] = MSG_HANDLE_DB_T(new db_center_msg::proto_name(), &func_route_t::fun_name); \
		}                                                                                             \
	}
#define BIND_PROTO_CMD_NO_CB(cmd, fun_name, proto_name)                                                      \
	{                                                                                                        \
		if (m_msgMap.end() != m_msgMap.find(cmd))                                                            \
		{                                                                                                    \
			ALERT_LOG("cmd inster err![0x%x]", cmd);                                                         \
			exit(0);                                                                                         \
		}                                                                                                    \
		else                                                                                                 \
		{                                                                                                    \
			m_msgMap[cmd] = MSG_HANDLE_DB_T(new db_center_msg::proto_name(), &func_route_t::fun_name, false); \
		}                                                                                                    \
	}

#undef BIND_PROTO_CMD
#undef BIND_PROTO_CMD_NO_CB
}
