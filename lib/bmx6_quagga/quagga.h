/*
 * Copyright (c) 2010  Axel Neumann
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/*

BGP/Quagga plugin inspired by:
http://www.nongnu.org/quagga/zhh.html
http://www.mail-archive.com/b.a.t.m.a.n@lists.open-mesh.org/msg00529.html
https://dev.openwrt.org/browser/packages/net/quagga/patches/120-quagga_manet.patch
https://github.com/zioproto/quagga-manet/commits/olsrpatch-0.99.21
http://olsr.org/git/?p=olsrd.git;a=tree;f=lib/quagga;h=031772d0be26605f97bc08b10694c8429252921f;hb=04ab230c75111be01b86c419c54f48dc6f6daa2e
 */


/*
 * check this patch from olsr-0.6.3/src/lib/quagga/patches/quagga-0.99.32.diff
 
diff --git a/zebra/rt_netlink.c b/zebra/rt_netlink.c
index 5909131..92288bd 100644
--- a/zebra/rt_netlink.c
+++ b/zebra/rt_netlink.c
@@ -1623,6 +1623,9 @@ netlink_route_multipath (int cmd, struct prefix *p, struct rib *rib,
                         addattr_l (&req.n, sizeof req, RTA_PREFSRC,
 				 &nexthop->src.ipv4, bytelen);

+		      if (rib->type == ZEBRA_ROUTE_OLSR)
+			req.r.rtm_scope = RT_SCOPE_LINK;
+







diff --git a/zebra/zebra_rib.c b/zebra/zebra_rib.c
@@ -381,6 +384,18 @@ nexthop_active_ipv4 (struct rib *rib, struct nexthop *nexthop, int set,

 	      return 1;
 	    }
+	  else if (match->type == ZEBRA_ROUTE_OLSR)
+	    {
+	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
+		if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB)
+		    && newhop->type == NEXTHOP_TYPE_IFINDEX)
+		  {
+		    if (nexthop->type == NEXTHOP_TYPE_IPV4)
+		      nexthop->ifindex = newhop->ifindex;
+		    return 1;
+		  }
+	      return 0;
+	    }
 	  else if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_INTERNAL))
 	    {
 	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
@@ -483,6 +498,18 @@ nexthop_active_ipv6 (struct rib *rib, struct nexthop *nexthop, int set,

 	      return 1;
 	    }
+	  else if (match->type == ZEBRA_ROUTE_OLSR)
+	    {
+	      for (newhop = match->nexthop; newhop; newhop = newhop->next)
+		if (CHECK_FLAG (newhop->flags, NEXTHOP_FLAG_FIB)
+		    && newhop->type == NEXTHOP_TYPE_IFINDEX)
+		  {
+		    if (nexthop->type == NEXTHOP_TYPE_IPV6)
+		      nexthop->ifindex = newhop->ifindex;
+		    return 1;
+		  }
+	      return 0;
+	    }
 	  else if (CHECK_FLAG (rib->flags, ZEBRA_FLAG_INTERNAL))
 	    {
 	      for (newhop = match->nexthop; newhop; newhop = newhop->next)

*/

///////////////////////////////////////////////////////////////////////////////

#define ZEBRA_VERSION2 2


///////////////////////////////////////////////////////////////////////////////
// from quagga/config.h
#define ZEBRA_SERV_PATH "/var/run/quagga/zserv.api" // "/var/run/quagga/zserv.api"



///////////////////////////////////////////////////////////////////////////////
// from quagga/lib/route_types.h (BMX6 patched: quagga/lib/route_types.txt):

/* Zebra route types. */
#define ZEBRA_ROUTE_SYSTEM               0
#define ZEBRA_ROUTE_KERNEL               1
#define ZEBRA_ROUTE_CONNECT              2
#define ZEBRA_ROUTE_STATIC               3
#define ZEBRA_ROUTE_RIP                  4
#define ZEBRA_ROUTE_RIPNG                5
#define ZEBRA_ROUTE_OSPF                 6
#define ZEBRA_ROUTE_OSPF6                7
#define ZEBRA_ROUTE_ISIS                 8
#define ZEBRA_ROUTE_BGP                  9
#define ZEBRA_ROUTE_HSLS                 10
#define ZEBRA_ROUTE_OLSR                 11
#define ZEBRA_ROUTE_BATMAN               12
#define ZEBRA_ROUTE_BMX6                 13
#define ZEBRA_ROUTE_BABEL                14
#define ZEBRA_ROUTE_MAX                  15


/* BMX6 route's types: look at hna.h. */



struct zapi_route_dict {
        uint8_t bmx2Zebra;
        uint8_t zebra2Bmx;
};


#define set_zapi_rt_dict( B, Z ) do { \
        zapi_rt_dict[ B ].bmx2Zebra = Z; \
        zapi_rt_dict[ Z ].zebra2Bmx = B; \
} while (0)

///////////////////////////////////////////////////////////////////////////////
// from quagga/lib/zebra.h:


/* Marker value used in new Zserv, in the byte location corresponding
 * the command value in the old zserv header. To allow old and new
 * Zserv headers to be distinguished from each other.
 */
#define ZEBRA_HEADER_MARKER              255

/* default zebra TCP port for zclient */
#define ZEBRA_PORT			2600





/* Zebra message types. */
#define ZEBRA_INTERFACE_ADD                1
#define ZEBRA_INTERFACE_DELETE             2
#define ZEBRA_INTERFACE_ADDRESS_ADD        3
#define ZEBRA_INTERFACE_ADDRESS_DELETE     4
#define ZEBRA_INTERFACE_UP                 5
#define ZEBRA_INTERFACE_DOWN               6
#define ZEBRA_IPV4_ROUTE_ADD               7
#define ZEBRA_IPV4_ROUTE_DELETE            8
#define ZEBRA_IPV6_ROUTE_ADD               9
#define ZEBRA_IPV6_ROUTE_DELETE           10
#define ZEBRA_REDISTRIBUTE_ADD            11
#define ZEBRA_REDISTRIBUTE_DELETE         12
#define ZEBRA_REDISTRIBUTE_DEFAULT_ADD    13
#define ZEBRA_REDISTRIBUTE_DEFAULT_DELETE 14
#define ZEBRA_IPV4_NEXTHOP_LOOKUP         15
#define ZEBRA_IPV6_NEXTHOP_LOOKUP         16
#define ZEBRA_IPV4_IMPORT_LOOKUP          17
#define ZEBRA_IPV6_IMPORT_LOOKUP          18
#define ZEBRA_INTERFACE_RENAME            19
#define ZEBRA_ROUTER_ID_ADD               20
#define ZEBRA_ROUTER_ID_DELETE            21
#define ZEBRA_ROUTER_ID_UPDATE            22
#define ZEBRA_HELLO                       23
#define ZEBRA_MESSAGE_MAX                 24

/* Subsequent Address Family Identifier. */
#define SAFI_UNICAST              1
#define SAFI_MULTICAST            2
#define SAFI_RESERVED_3           3
#define SAFI_MPLS_VPN             4
#define SAFI_MAX                  5


/* Zebra's family types. */
#define ZEBRA_FAMILY_IPV4                1
#define ZEBRA_FAMILY_IPV6                2
#define ZEBRA_FAMILY_MAX                 3

/* Error codes of zebra. */
#define ZEBRA_ERR_NOERROR                0
#define ZEBRA_ERR_RTEXIST               -1
#define ZEBRA_ERR_RTUNREACH             -2
#define ZEBRA_ERR_EPERM                 -3
#define ZEBRA_ERR_RTNOEXIST             -4
#define ZEBRA_ERR_KERNEL                -5

/* Zebra message flags */
#define ZEBRA_FLAG_INTERNAL           0x01
#define ZEBRA_FLAG_SELFROUTE          0x02
#define ZEBRA_FLAG_BLACKHOLE          0x04
#define ZEBRA_FLAG_IBGP               0x08
#define ZEBRA_FLAG_SELECTED           0x10
#define ZEBRA_FLAG_CHANGED            0x20
#define ZEBRA_FLAG_STATIC             0x40
#define ZEBRA_FLAG_REJECT             0x80

/* Zebra nexthop flags. */
#define ZEBRA_NEXTHOP_IFINDEX            1
#define ZEBRA_NEXTHOP_IFNAME             2
#define ZEBRA_NEXTHOP_IPV4               3
#define ZEBRA_NEXTHOP_IPV4_IFINDEX       4
#define ZEBRA_NEXTHOP_IPV4_IFNAME        5
#define ZEBRA_NEXTHOP_IPV6               6
#define ZEBRA_NEXTHOP_IPV6_IFINDEX       7
#define ZEBRA_NEXTHOP_IPV6_IFNAME        8
#define ZEBRA_NEXTHOP_BLACKHOLE          9


///////////////////////////////////////////////////////////////////////////////
// from quagga/lib/zclient.h:
// see also: quagga/lib/zclient.c: zapi_ipv4_route()
//           quagga/zebra/zserv.c: zread_ipv4_add()
//           olsrd-0.6.3/lib/quagga/src/quagga.c: zerba_addroute()

/* Zebra API message flag. */
#define ZAPI_MESSAGE_NEXTHOP  0x01
#define ZAPI_MESSAGE_IFINDEX  0x02
#define ZAPI_MESSAGE_DISTANCE 0x04
#define ZAPI_MESSAGE_METRIC   0x08

/* Zserv protocol message header */
struct zapiV2_header {
        uint16_t length;
        uint8_t marker; // always set to 255 in new zserv.
        uint8_t version;
        uint16_t command;
} __attribute__((packed));

/*
struct zapiV2_route_write0 {
        struct zapiV2_header hdr;
        uint8_t type;
        uint8_t flags;
        uint8_t message;
        uint16_t safi; // zclient.h uses uint8_t here  !! This field only exist in quagga/zebra/zserv.c:zread_ipvX_add/del()
        uint8_t prefixlen;
        uint8_t prefix[];
};

struct zapiV2_route4_write1 {
        uint8_t nexthop_num;
        uint8_t nexthop_type_af;
        IP4_T nexthop;
        uint8_t nexthop_type_ifidx;
        uint32_t ifidx;
        uint8_t distance;
        uint32_t metric;
} __attribute__((packed));

struct zapiV2_route6_write1 {
        uint8_t nexthop_num;
        uint8_t nexthop_type_af;
        IP6_T nexthop;
        uint8_t nexthop_type_ifidx;
        uint32_t ifidx;
        uint8_t distance;
        uint32_t metric;
} __attribute__((packed));
*/

struct zroute_key {
        struct net_key net;
        IPX_T via;
        uint32_t ifindex;
        uint32_t metric;
        uint8_t ztype;
        uint8_t distance;
};

struct zroute_node {
        struct zroute_key k;

        uint8_t flags;
        uint8_t message;
        int8_t cnt;
        uint8_t old;
};

struct redist_out_key {
        uint8_t bmx6_route_type;
        FMETRIC_U8_T bandwidth;
        struct net_key net;
        uint8_t must_be_one; // to find_next route_type and bandwidth if net is zero
};

struct redist_out_node {
        struct redist_out_key k;
        uint8_t minAggregatePrefixLen;
        uint8_t old;
        uint8_t new;
};


//struct zdata {
//        struct zapiV2_header* hdr;
//        uint16_t len;
//        uint16_t cmd;
//};


///////////////////////////////////////////////////////////////////////////////


#define ZSOCK_RECONNECT_TO 1000


struct zebra_cfg {
        char unix_path[MAX_PATH_SIZE];
        int32_t port;
        IPX_T ipX;
        int32_t socket;
        char *zread_buff;
        uint16_t zread_buff_len;
        uint16_t zread_len;
        uint32_t bmx6_redist_bits_new;
        uint32_t bmx6_redist_bits_old;

        //...
};

struct zsock_write_node {
	struct list_node list;
	char *zpacket;
        uint16_t send;
};

struct zdata {
	struct list_node list;
//	char *zpacket;
        struct zapiV2_header* hdr;
        uint16_t len;
        uint16_t cmd;
};


#define ARG_ZAPI_DIR "zebraPath"


/*
#define ARG_EXPORT           "export"

#define ARG_EXPORT_UHNA      "unicastHna"
#define DEF_EXPORT_UHNA      0

#define ARG_EXPORT_RTYPE_BMX "bmx6"
#define DEF_EXPORT_RTYPE_BMX 1

#define ARG_EXPORT_ONLY   "exportOnly"
#define DEF_EXPORT_ONLY   0
#define MIN_EXPORT_ONLY   0
#define MAX_EXPORT_ONLY   1
*/

#define ARG_REDIST        "redistribute"

#define ARG_QUAGGA_NET    "network"
#define ARG_QUAGGA_BW     "bandwidth"

#define ARG_QUAGGA_METRIC "metric"
#define DEF_QUAGGA_METRIC 0
#define MIN_QUAGGA_METRIC 0
#define MAX_QUAGGA_METRIC INT32_MAX

#define ARG_QUAGGA_DISTANCE "distance"
#define DEF_QUAGGA_DISTANCE 0
#define MIN_QUAGGA_DISTANCE 0
#define MAX_QUAGGA_DISTANCE UINT8_MAX

#define MIN_QUAGGA_PREFIX 0
#define MAX_QUAGGA_PREFIX 129
#define TYP_QUAGGA_PREFIX_NET 129 //assumes prefix from ARG_TUN_OUT_NET

#define ARG_QUAGGA_PREFIX_MIN "minPrefixLen"
#define DEF_QUAGGA_PREFIX_MIN TYP_QUAGGA_PREFIX_NET

#define ARG_QUAGGA_PREFIX_MAX "maxPrefixLen"
#define DEF_QUAGGA_PREFIX_MAX 128

#define ARG_QUAGGA_AGGREGATE "aggregatePrefixLen"
#define MIN_QUAGGA_AGGREGATE 0
#define MAX_QUAGGA_AGGREGATE 128
#define DEF_QUAGGA_AGGREGATE 0

//#define MIN_QUAGGA_RTYPE_ENABLED 0
//#define MAX_QUAGGA_RTYPE_ENABLED 1
//#define DEF_QUAGGA_RTYPE_ENABLED 0


#define ARG_QUAGGA_HYSTERESIS "hysteresis"
#define DEF_QUAGGA_HYSTERESIS 20
#define MIN_QUAGGA_HYSTERESIS 0
#define MAX_QUAGGA_HYSTERESIS MIN(100000, (UMETRIC_MULTIPLY_MAX - 100))

#define NETWORK_NAME_LEN 32

struct redistr_opt_node {
        char nameKey[NETWORK_NAME_LEN];
        struct net_key net;
        uint32_t bmx6_redist_bits;
        uint32_t hysteresis;
        uint8_t netPrefixMin;
        uint8_t netPrefixMax;
        uint8_t minAggregatePrefixLen;
        FMETRIC_U8_T bandwidth;
};

/*
struct export_opt_node {
        char nameKey[NETWORK_NAME_LEN];
        struct net_key net;
	uint8_t exportRTBmx6;
	uint8_t exportUHna;
	uint8_t exportOnly;
        uint8_t netPrefixMin;
        uint8_t netPrefixMax;
	uint8_t distance;
        uint32_t metric;
	uint32_t hysteresis;
};
*/
