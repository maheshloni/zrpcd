/* zrpc core structures and API
 * Copyright (c) 2016 6WIND,
 *
 * This file is part of ZRPC daemon.
 *
 * See the LICENSE file.
 */
#ifndef _ZRPC_VPNSERVICE_H
#define _ZRPC_VPNSERVICE_H

#include "zrpcd/zrpc_os_wrapper.h"
#include "zrpcd/zrpc_util.h"
#include "zrpcd/zrpc_bgp_capnp.h"

#define ZRPC_LISTEN_PORT	 7644
#define ZRPC_NOTIFICATION_PORT 6644
#define ZRPC_CLIENT_ADDRESS "0.0.0.0"

#define ZMQ_SOCK "ipc:///tmp/qzc-vpn2bgp"
#define ZMQ_NOTIFY "ipc:///tmp/qzc-notify"

#define BGPD_ARGS_STRING_1  "-p"
#define BGPD_ARGS_STRING_3  "-Z"

#define BGPD_PATH_BGPD_PID "/opt/quagga/var/run/quagga/bgpd.pid"
#define BGPD_PATH_QUAGGA   "/opt/quagga"

#define ZRPC_CONFIG_FILE   "zrpcd.conf"

#define ZRPC_DEFAULT_LOG_FILE "/opt/quagga/var/log/quagga/zrpcd.init.log"
#define ZRPC_DEFAULT_LOG_LEVEL "debugging"

struct thread;

struct zrpc_vpnservice_client
{
  ThriftProcessor *processor;
  ThriftTransport *transport;
  ThriftBufferedTransport *transport_buffered;
  ThriftProtocol *protocol;
  ThriftServer *server;
  ThriftSimpleServer *simple_server;
};

struct zrpc_vpnservice_bgp_context
{
  uint32_t asNumber;
  gint32 proc;
  char *logFile;
  char *logLevel;
  uint8_t multipath_on[AFI_MAX][SAFI_MAX];
};

/* zrpc cache contexts */
struct zrpc_vpnservice_cache_bgpvrf
{
  uint64_t bgpvrf_nid;
  struct zrpc_rd_prefix outbound_rd;
  struct zrpc_vpnservice_cache_bgpvrf *next;
  bgp_layer_type_t ltype;
  uint8_t afc[ADDRESS_FAMILY_MAX][SUBSEQUENT_ADDRESS_FAMILY_MAX];
};

struct zrpc_vpnservice_cache_peer
{
  uint64_t peer_nid;
  uint32_t asNumber;
  char *peerIp;
  struct zrpc_vpnservice_cache_peer *next;
  uint8_t enableAddressFamily[ADDRESS_FAMILY_MAX][SUBSEQUENT_ADDRESS_FAMILY_MAX];
};

struct zrpc_vpnservice
{
  /* configuration part */
  /* zrpc listen port number.  */
  u_int16_t  zrpc_listen_port;

  /* zrpc notification port number.  */
  u_int16_t  zrpc_notification_port;

  /* zrpc BGP Contexts */
  ThriftServerTransport *bgp_configurator_server_transport;
  BgpConfiguratorProcessor *bgp_configurator_processor;
  InstanceBgpConfiguratorHandler *bgp_configurator_handler;

  /* zrpc Update Contexts */
  BgpUpdaterIf *bgp_updater_client;
  struct thread *bgp_updater_client_thread;
  ThriftSocket *bgp_updater_socket;
  ThriftFramedTransport *bgp_updater_transport;
  ThriftProtocol *bgp_updater_protocol;

  /* bgp context */
  struct zrpc_vpnservice_bgp_context *bgp_context;

  /* CapnProto Path */
  char      *zmq_sock;

  /* CapnProto Subscribe Path */
  char      *zmq_subscribe_sock;

  /* BGPD binay execution path */
  char     *bgpd_execution_path;

  /* QZC internal contexts */
  struct qzcclient_sock *qzc_sock;
  struct qzcclient_sock *qzc_subscribe_sock;
  
  /* zrpc cache context for VRF */
  struct zrpc_vpnservice_cache_bgpvrf *bgp_vrf_list;
  struct zrpc_vpnservice_cache_peer *bgp_peer_list;
  struct zrpc_vpnservice_cache_bgpvrf *bgp_get_routes_list;

  /* bgp updater statistics */
  u_int32_t bgp_update_lost_msgs;
  u_int32_t bgp_update_monitor;
  u_int32_t bgp_update_retries;
  u_int32_t bgp_update_total;
  u_int32_t bgp_update_thrift_lost_msgs;
};

void zrpc_vpnservice_terminate(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_terminate_thrift_bgp_configurator_server(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_terminate_thrift_bgp_updater_client(struct zrpc_vpnservice *setup);
gboolean zrpc_vpnservice_setup_thrift_bgp_updater_client (struct zrpc_vpnservice *setup);
void zrpc_vpnservice_setup_thrift_bgp_configurator_server(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_setup(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_setup_bgp_cache(struct zrpc_vpnservice *ctxt);

void zrpc_vpnservice_get_context (struct zrpc_vpnservice **setup);
u_int16_t zrpc_vpnservice_get_thrift_bgp_configurator_server_port (struct zrpc_vpnservice *setup);
void zrpc_vpnservice_set_thrift_bgp_updater_client_port (struct zrpc_vpnservice *setup, \
                                                            u_int16_t thrift_notif_port);
u_int16_t zrpc_vpnservice_get_thrift_bgp_updater_client_port (struct zrpc_vpnservice *setup);
void zrpc_vpnservice_set_thrift_bgp_configurator_server_port (struct zrpc_vpnservice *setup, \
                                                                 u_int16_t thrift_listen_port);
void zrpc_vpnservice_setup_client(struct zrpc_vpnservice_client *peer,\
                                     struct zrpc_vpnservice *setup,  \
                                     ThriftTransport *transport);

void zrpc_vpnservice_terminate_client(struct zrpc_vpnservice_client *peer);
void zrpc_vpnservice_terminate_qzc(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_setup_qzc(struct zrpc_vpnservice *setup);
struct zrpc_vpnservice_bgp_context *zrpc_vpnservice_get_bgp_context(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_setup_bgp_context(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_terminate_bgp_context(struct zrpc_vpnservice *setup);
void zrpc_vpnservice_terminate_bgpvrf_cache (struct zrpc_vpnservice *setup);
gboolean zrpc_vpnservice_set_bgp_context_multipath (struct zrpc_vpnservice_bgp_context *bgp,
                                                    address_family_t afi, subsequent_address_family_t safi,
                                                    uint8_t on, gint32* _return, GError **error);

#endif /* _ZRPC_VPNSERVICE_H */
