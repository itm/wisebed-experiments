#include <isense/os.h>
#include <isense/task.h>
#include <isense/uart.h>
#include <isense/time.h>
#include <isense/isense.h>
#include <isense/util/util.h>
#include <isense/application.h>
#include <isense/protocols/ip/coap/coap_server.h>
#include <isense/protocols/ip/coap/coap_client.h>
#include <isense/protocols/ip/version6/new_dymo.h>
#include <isense/modules/ethernet_module/enc28j60.h>
#include <isense/protocols/ip/version6/ipv6_address.h>
#include <isense/protocols/ip/coap/resources/spitfire/resources_s.h>
#include <isense/protocols/ip/version6/sixlowpan_network_interface.h>
#include <isense/protocols/ip/version6/static_routing.h>
//#include <isense/protocols/routing/dymo_low.h>

//Uncomment for Gateway
//#define GATEWAY

#define SET_UART0_1MBIT 0
#define SET_UART1_1MBIT 0

//Environment module
#define SET_ENV 1
//Security module
#define SET_SSM 0

using namespace isense;
using namespace ip_stack;

enum userdata
{
	ud_print_lists = 1,
	ud_register_node = 2
};

class CoAPGateway :
	public Application,
	public Task,
	public UartPacketHandler,
	public CoAPCallback
{
	
public:
	CoAPGateway(isense::Os& os);
	~CoAPGateway() ;
	void boot (void) ;
	void execute( void* userdata ) ;
	void handle_uart_packet( uint8 type, uint8* buf, uint8 length );
	void print_lists( uint8 interface_mask , uint8 list_mask );
	void coap_request_callback( CoAPresponse* response, uint8 response_code, Buffer payload, bool notification );

private:
	IPv6* ip_;
	SixlowpanNetworkInterface * ip_eth0_;
	IPv6NetworkInterface* ip_eth1_;
	UdpSocket* udpSocket_;
	CoAPClient* client_;
};

//----------------------------------------------------------------------------
CoAPGateway::
	CoAPGateway(isense::Os& os) :
		isense::Application(os)
	{}

//----------------------------------------------------------------------------
#if defined (ISENSE_SHAWN) || not defined (ISENSE_AVOID_EMPTY_DESTRUCTORS)
CoAPGateway::
	~CoAPGateway()
	{
	}
#endif
//----------------------------------------------------------------------------
void
	CoAPGateway::
	boot(void)
	{
		os_.allow_sleep(false);
		os_.allow_doze(false);

		#ifdef GATEWAY
		   vAHI_HighPowerModuleEnable(true, true);
		   #ifdef ISENSE_JENNIC_JN5148
			vAHI_ETSIHighPowerModuleEnable(true);
		   #endif
		#endif

		os_.radio().hardware_radio().set_channel(14);

		#ifdef GATEWAY
			os_.debug("Booting IPv6 Gateway demo application");
		#else
			os_.debug("Booting IPv6 Router demo application");
		#endif

		/*
		 * Initialize the UART
		 */
		#if SET_UART0_1MBIT
			os_.uart(0);
			vAHI_UartSetBaudDivisor( 0, 1 );
			vAHI_UartSetClocksPerBit( 0, 15 );
		#endif

		#if SET_UART1_1MBIT
			os_.uart(1);
			vAHI_UartSetBaudDivisor( 1, 1 );
			vAHI_UartSetClocksPerBit( 1, 15 );
			//os_.set_log_mode( ISENSE_LOG_MODE_UART1 );
		#endif

		ip_ = new IPv6( os_ );
		char address[40];

		/*
		 * Create SixlowpanNetworkInterface and add it to the IPv6 layer
		 */
		#ifdef GATEWAY
//			Routing* link_layer_routing_ = new DymoLow(os_);
//			link_layer_routing_->enable();
//			ip_eth0_ = new SixlowpanNetworkInterface(os_, *ip_, os_.radio() , 8 , link_layer_routing_ , true);
//			ip_eth0_->enable_router( true );
			ip_eth0_ = new SixlowpanNetworkInterface( os_, *ip_, os_.radio(), 8, true );
		#else
//			Routing* link_layer_routing_ = new DymoLow(os_);
//			link_layer_routing_->enable();
//			ip_eth0_ = new SixlowpanNetworkInterface(os_, *ip_, os_.radio() , 8 , link_layer_routing_ , false);

			ip_eth0_ = new SixlowpanNetworkInterface( os_, *ip_, os_.radio(), 8, false );
		#endif

		uint8 eth0_index = ip_->add_interface( ip_eth0_ );
		os_.debug("Link local address of eth0 %s", (ip_eth0_->get_link_local_unicast_ip_address())->to_string( address, 40 ));

		/*
		 * Set IPv6 prefix of lowpan network. You should alter this to a prefix routed to this device
		 */
		#ifdef GATEWAY
			IPv6Address lowpan_prefix( (uint8[]){0x20,0x01,0x06,0x38 , 0x07,0x0a,0xc0,0x05 , 0,0,0,0 , 0,0,0,0} );
			ip_eth0_->add_prefix( lowpan_prefix, 64, PREFIX_FLAG_AUTONOMOUS | PREFIX_FLAG_ADVERTISE );
		#endif

		ip_eth0_->enable_router( true );
			
		/*
		 * Add route over protocol
		 */
		//	IpRouting* dymo = new Dymo( *ip_, eth0_index, 20 );
		//	ip_->add_routing( dymo );

		StaticRouting* static_routing = new StaticRouting(*ip_, eth0_index);

		#ifdef GATEWAY
			/*
			 * Create and initialize Ethernet
			 */
			Enc28J60* eth1 = new Enc28J60( os_, 0x001507201001ULL, 6500, true );
			eth1->enable();
			eth1->init();
			/*
			 * Create IPv6NetworkInterface out of eth0
			 */
			ip_eth1_ = new IPv6NetworkInterface( os_, *ip_, *eth1, 6 );
			/*
			 * Add IPv6NetworkInterface to IPv6 layer
			 */
			ip_->add_interface( ip_eth1_ );
			os_.debug("Link local address of eth1 %s", (ip_eth1_->get_link_local_unicast_ip_address())->to_string( address, 40 ));
			ip_eth1_->enable_router( true );
		#endif

		os_.uart(0).set_packet_handler( Uart::MESSAGE_TYPE_CUSTOM_IN_1 ,this );
		os_.uart(0).enable_interrupt( true );

		/*
		 * Create CoAP server
		 */
		CoAPServer* server = new CoAPServer( os_, ip_->udp() );
		hack = ip_;

		MemoryResource_s* _memory = new MemoryResource_s( *server );
		_memory->initWithLink("</memory>;rt=\"MemoryB\";title=\"Memory Resource\";ct=0;obs");
		server->add_resource( _memory );
		os_.debug("Memory resource added!");

		#if SET_SSM
			AccelerometerResource_s* _acc = new AccelerometerResource_s( *server );
			_acc->initWithLink("</acc>;rt=\"AccB\";title=\"Accelerometer Resource\";ct=0;obs");
			server->add_resource( _acc );
			os_.debug("Accelerometer resource added!");

			PirResource_s* _pir = new PirResource_s( *server );
			_pir->initWithLink("</pir>;rt=\"PirD\";title=\"Pir Sensor\";ct=0;obs");
			server->add_resource( _pir );
			os_.debug("PIR resource added!");
		#endif

		#if SET_ENV
			LightResource_s* _light = new LightResource_s( *server );
			_light->initWithLink("</light>;rt=\"LightLux\";title=\"Light Sensor\";ct=0;obs");
			server->add_resource( _light );
			os_.debug("Light resource added!");

			TempResource_s* _temp = new TempResource_s( *server );
			_temp->initWithLink("</temp>;rt=\"TempC\";title=\"Temperature Sensor\";ct=0");
			server->add_resource( _temp );
			os_.debug("Temperature resource added!");
		#endif

//		os_.srand(isense::JennicOs::jennic_os->get_seed());
//		uint16 addr = os_.id()&0xff;
//		os_.add_task_in( Time(os_.rand(addr%10 ), 0), this, (void*)ud_register_node );

			  uint16 shortID = os_.id() & 0xffff;
			  if( shortID == 0x211c ){
			   os_.add_task_in( Time(3, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2114 ){
			   os_.add_task_in( Time(7, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2104 ){
			   os_.add_task_in( Time(11, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2118 ){
			   os_.add_task_in( Time(15, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2120 ){
			   os_.add_task_in( Time(19, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2144 ){
			   os_.add_task_in( Time(23, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2140 ){
			   os_.add_task_in( Time(27, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2108 ){
			   os_.add_task_in( Time(31, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2100 ){
			   os_.add_task_in( Time(35, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x210c ){
			   os_.add_task_in( Time(39, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2124 ){
			   os_.add_task_in( Time(43, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2134 ){
			   os_.add_task_in( Time(47, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2130 ){
			   os_.add_task_in( Time(51, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x212c ){
			   os_.add_task_in( Time(55, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2138 ){
			   os_.add_task_in( Time(59, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x213c ){
			   os_.add_task_in( Time(63, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2110 ){
			   os_.add_task_in( Time(67, 0), this, (void*)ud_register_node );
			  } else if( shortID == 0x2128 ){
			   os_.add_task_in( Time(71, 0), this, (void*)ud_register_node );
			  }
			  else{
				  os_.add_task_in( Time(3, 0), this, (void*)ud_register_node );
			  }

		//os_.add_task_in( Time(os_.rand( os_.id()%10 ), 0), this, (void*)ud_register_node );
		os_.debug("MAC-Address: %d", os_.id());
 	}

//----------------------------------------------------------------------------

void
	CoAPGateway::
	execute( void* userdata )
	{
		if( userdata == (void*)ud_print_lists )
		{
			print_lists(0xff, 0xff);
		} else if( userdata == (void*)ud_register_node )
		{
			os_.fatal("Sending here i am");
			//Send registration message to server
			client_ = new CoAPClient( os_, ip_->udp(), 5682);
			
			//IPv6Address rem_address = IPv6Address::from_string("2001:638:205:1978:a00:27ff:fea3:7bac");
			//Global unicast Windows
			IPv6Address rem_address = IPv6Address::from_string("2001:638:70a:b157::c002:c002");
			
			//TODO: Multicast!
			//IPv6Address rem_address = IPv6Address::from_string("ff0e::2");
			rem_address.interface_id_ = 0;
			const char path[] = "/here_i_am";
			const char query[] = "";

			client_->coap_request( &rem_address, 5683, (uint8) 11, (const char*) &path, (uint8) 0, (const char*) &query, (uint8) 1, (uint16) 0, (const uint8*) 0, this, unsubscribe, true);
		}
	}

//----------------------------------------------------------------------------

void
	CoAPGateway::
	handle_uart_packet( uint8 type, uint8* buf, uint8 length )
{
	if ( type == Uart::MESSAGE_TYPE_CUSTOM_IN_1 ) 
	{
		os_.add_task( this , (void*) ud_print_lists );
	}
}

void
	CoAPGateway::
	print_lists( uint8 interface_mask , uint8 list_mask )
{
	if ( ip_ != NULL ) 
	{
		for (uint8 i = 0; i < 8; i++) 
	{
			if (interface_mask & 0x01) 
			{
				IPv6NetworkInterface* interface = ip_->get_interface(i);
				if ( interface != NULL ) 
				{
					char c[100];
					os_.debug("%s" , interface->to_string(c,99));
					if ( list_mask & 0x01 ) 
					{
						interface->print_address_list();
					}
					
					if ( list_mask & 0x02 ) 
					{
						interface->print_prefix_list();
					}
					
					if ( list_mask & 0x04 ) 
					{
						interface->print_default_router_list();
					}
					
					if ( list_mask & 0x08 ) 
					{
						interface->print_neighbor_cache();
					}
					os_.debug("");
				}
			}
			interface_mask >>= 1;
		}
		os_.debug("");
	}
}

void
	CoAPGateway::
	coap_request_callback( CoAPresponse* response, uint8 response_code, Buffer payload, bool notification )
{
	os_.fatal("CoAP Callback");
}

isense::Application* 
	application_factory(isense::Os& os)
{
	return new CoAPGateway(os);
}

