#include <xr_include.h>
#include <xr_log.h>
#include <mysql_if.h>
#include <config.h>
#include <xr_error.h>

#include "func_rount.h"

static xr_mysql::mysql_if_t *g_db = NULL;
func_route_t *g_route_func = NULL;

extern "C" int on_init()
{
	if (xr_server::is_parent())
	{
		DEBUG_LOG("======parent start======");
	}
	else
	{
		DEBUG_LOG("======child start======");

		g_db = new xr_mysql::mysql_if_t(xr_server::g_config->get_val_str("mysql", "ip"),
										xr_server::g_config->get_val_str("mysql", "user"),
										xr_server::g_config->get_val_str("mysql", "passwd"),
										::atoi(xr_server::g_config->get_val_str("mysql", "port").c_str()),
										true, xr_server::g_config->get_val_str("mysql", "unix_socket").c_str());
		g_db_name_pre = xr_server::g_config->get_val_str("mysql", "db_name_pre");

		g_route_func = new func_route_t(g_db);
	}
	return 0;
}

extern "C" int on_fini()
{
	if (xr_server::is_parent())
	{
		DEBUG_LOG("======parent done======");
	}
	else
	{
		DEBUG_LOG("======child done======");
		SAFE_DELETE(g_db);
		SAFE_DELETE(g_route_func);
	}
	return 0;
}

extern "C" void on_events()
{
}

extern "C" int on_get_pkg_len(xr::tcp_peer_t *peer,
							  const void *data, uint32_t len)
{
	if (len < xr_server::proto_head_t::PROTO_HEAD_LEN)
	{
		return 0;
	}

	char *c = (char *)data;
	xr_server::PROTO_LEN pkg_len = (xr_server::PROTO_LEN)(*(xr_server::PROTO_LEN *)c);

	if (pkg_len < xr_server::proto_head_t::PROTO_HEAD_LEN || pkg_len >= xr_server::g_config->page_size_max)
	{
		ERROR_LOG("pkg len error %u", pkg_len);
		return xr::ERROR::DISCONNECT_PEER;
	}

	if (len < pkg_len)
	{
		return 0;
	}

	return pkg_len;
}

extern "C" int on_cli_pkg(xr::tcp_peer_t *peer, const void *data, uint32_t len)
{
	g_route_func->do_handle_dispatcher(data, len, peer);
	return 0;
}

extern "C" void on_srv_pkg(xr::tcp_peer_t *peer, const void *data, uint32_t len)
{
}

extern "C" void on_cli_conn(xr::tcp_peer_t *peer)
{
	DEBUG_LOG("fd:%d connected", peer->fd);
}

extern "C" void on_cli_conn_closed(int fd)
{
	CRITI_LOG("fd:%d", fd);
}

extern "C" void on_svr_conn_closed(int fd)
{
}

extern "C" void on_mcast_pkg(const void *data, int len)
{
}

extern "C" void on_addr_mcast_pkg(uint32_t id, const char *name,
								  const char *ip, uint16_t port, const char *data, int flag)
{
	//	TRACE_LOG("id:%u, name:%s, ip:%s, port:%u, flag:%u", id, name, ip, port, flag);
}

extern "C" void on_udp_pkg(int fd, const void *data, int len,
						   struct sockaddr_in *from, socklen_t fromlen)
{
}