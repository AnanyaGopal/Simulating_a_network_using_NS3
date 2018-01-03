/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation;
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/output-stream-wrapper.h"


	using namespace ns3;

	NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

	int main (int argc, char *argv[])
	{
	bool verbose = true;
	uint32_t nCsma = 15;
	uint32_t nWifi = 8;
	uint32_t nSpokes = 15;

	// NetAnim component
	std::string animFile = "csmap2p.xml" ;  // Name of file for animation output
	std::string transportProt = "Tcp";
	std::string socketType;

	CommandLine cmd;
	cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
	cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
	cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
	cmd.AddValue ("transportProt", "Transport protocol to use: Tcp, Udp", transportProt);
	cmd.AddValue ("nSpokes", "Number of nodes to place in the star", nSpokes);

	cmd.Parse (argc,argv);

	if (transportProt.compare ("Tcp") == 0)
	{
		socketType = "ns3::TcpSocketFactory";
	}
	else
	{
		socketType = "ns3::UdpSocketFactory";
	}

	if (verbose)
	{
		LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	// Conditional operator - if nodes are 0, the nodes =1, or else equal to the number specified above.
	nCsma = nCsma == 0 ? 1 : nCsma;

	
	// ------------- Creating systems that will communicate with each other ------------ //
	// Two routers --> Both routers have abilities to hold their own nodes.
	NodeContainer Routers;
	Routers.Create(4);

	// First set of systems. - > 15 nodes + one router (R1)
	NodeContainer csmaNodesR1;
	csmaNodesR1.Add (Routers.Get (0));
	csmaNodesR1.Create(nCsma);

	// Second set of systems - > 15 nodes + one router (R2)
	NodeContainer csmaNodesR2;
	csmaNodesR2.Add (Routers.Get (1));
	csmaNodesR2.Create (nCsma);
	// -----------END OF Creating systems that will communicate with each other ------------ //

	// We need point to point connection between both computers
	PointToPointHelper pointToPointBetweenRouters;
	pointToPointBetweenRouters.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPointBetweenRouters.SetChannelAttribute ("Delay", StringValue ("2ms"));

// We need point to point connection between both computers
	PointToPointHelper failLink;
	failLink.SetDeviceAttribute ("DataRate", StringValue ("0.0001Mbps"));
	failLink.SetChannelAttribute ("Delay", StringValue ("1000ms"));

	// -----------End OF connecting both routers ------------ //


	// Establishing a csma connection in both routers and their nodes.
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
	csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install (csmaNodesR1);

	NetDeviceContainer csmaDevicesR2;
	csmaDevicesR2 = csma.Install (csmaNodesR2);

	// End of ------------Establishing a csma connection in both routers and their nodes.

	// wifiNetwork = Router(0) ---> AP(1)---> 15 WiFi nodes (2-17)

	NodeContainer wifiStaNodes;
	wifiStaNodes.Create(nWifi);
	
	NodeContainer wifiApNode;
	wifiApNode.Create(1);
	wifiApNode.Add(wifiStaNodes);
	
	//--------Need to install an Internet Stack (TCP, UDP, IP, etc.) on each of the nodes in the node container.
	
	// Installing stack on all containers except R4.
	InternetStackHelper stack;
	stack.Install(csmaNodesR1);
	stack.Install(csmaNodesR2);
	stack.Install(Routers.Get(2));
	stack.Install(Routers.Get(3));
	stack.Install (wifiApNode);

	
	Ipv4AddressHelper address;

	//--------Need to give base network addresses, and subnet mask to the point to point interface between routers ------ //
	/* ********************************************************************************************************************************  */
	/* * * R1 and R2 */
	
	
	NodeContainer subnet2;
	subnet2.Add(Routers.Get(0));
	subnet2.Add(Routers.Get(1)); 
	//NetDeviceContainer subnet2DEV = failLink.Install (subnet2);
	
	NetDeviceContainer subnet2DEV = pointToPointBetweenRouters.Install (subnet2);
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer Router12Interfaces = address.Assign (subnet2DEV);
	
	
	/* ********************************************************************************************************************************  */
	/* * * R1 and R3 */
	NodeContainer subnet13;
	subnet13.Add(Routers.Get(0));
	subnet13.Add(Routers.Get(2)); 
	NetDeviceContainer subnet13DEV = pointToPointBetweenRouters.Install (subnet13);
	address.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer Router13Interfaces = address.Assign (subnet13DEV);
	
	/* ********************************************************************************************************************************  */
	/* * * R1 and R4 */
	NodeContainer subnet14;
	subnet14.Add(Routers.Get(0));
	subnet14.Add(Routers.Get(3)); 
	NetDeviceContainer subnet14DEV = pointToPointBetweenRouters.Install (subnet14);
	address.SetBase ("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer Router14Interfaces = address.Assign (subnet14DEV);
	
	
	/* ********************************************************************************************************************************  */
	/* * * R2 and R3 */
	NodeContainer subnet23;
	subnet23.Add(Routers.Get(1));
	subnet23.Add(Routers.Get(2)); 
	NetDeviceContainer subnet23DEV = pointToPointBetweenRouters.Install (subnet23);
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer Router23Interfaces = address.Assign (subnet23DEV);

	/* ********************************************************************************************************************************  */
	/* * * R2 and R4 */
	NodeContainer subnet24;
	subnet24.Add(Routers.Get(1));
	subnet24.Add(Routers.Get(3)); 
	NetDeviceContainer subnet24DEV = pointToPointBetweenRouters.Install (subnet24);
	address.SetBase ("10.1.5.0", "255.255.255.0");
	Ipv4InterfaceContainer Router24Interfaces = address.Assign (subnet24DEV); 
	
	/* ********************************************************************************************************************************  */
	/* * * R3 and R4 */
	NodeContainer subnet34;
	subnet34.Add(Routers.Get(2));
	subnet34.Add(Routers.Get(3)); 
	NetDeviceContainer subnet34DEV = pointToPointBetweenRouters.Install (subnet34);
	address.SetBase ("10.1.6.0", "255.255.255.0");
	Ipv4InterfaceContainer Router34Interfaces = address.Assign (subnet34DEV);

	/* ********************************************************************************************************************************  */
	/* * * Network One */ 
	address.SetBase ("192.168.12.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaInterfaces;
	csmaInterfaces = address.Assign (csmaDevices);
	// Routers interface will get 192.168.12.4
	// Hosts will get 12.1-12.3

	/* ********************************************************************************************************************************  */
	/* * * Network Two */ 
	address.SetBase ("192.168.23.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaInterfacesR2;
	csmaInterfacesR2 = address.Assign (csmaDevicesR2);
	// Routers interface will get 192.168.23.4
	// Hosts will get 23.1-23.2

	/* ********************************************************************************************************************************  */
	/* * * Star network associated with R3 */ 

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
	PointToPointStarHelper star (nSpokes, pointToPoint);
	pointToPoint.SetQueue ("ns3::DropTailQueue", "Mode", StringValue ("QUEUE_MODE_PACKETS"), "MaxPackets", UintegerValue (1));

	InternetStackHelper internet;
	star.InstallStack (internet);

	star.AssignIpv4Addresses (Ipv4AddressHelper ("136.25.1.0", "255.255.255.0"));

	//Configuring Subnets
	//Create a node container to hold the nodes that belong to subnet1 and add star hub to router 1.
	NodeContainer subnet1;
	subnet1.Add(star.GetHub());
	subnet1.Add(Routers.Get(2));

	//Create a device conatiner to hold net devices installed on each node.
	NetDeviceContainer subnet1Devices = pointToPointBetweenRouters.Install(subnet1);

	//Configure the subnet address and mask
	address.SetBase("162.80.1.0","255.255.255.0");

	//Create an interface container to hold the ipv4 interface create and assign IP addresses to interface
	Ipv4InterfaceContainer subnetInterfaces = address.Assign(subnet1Devices);

	/* ********************************************************************************************************************************  */
	/* * * WiFi network associated with R4 */ 
	/* * * R1 and R2 */
	NodeContainer subnet4AP;
	subnet4AP.Add(Routers.Get(3));
	subnet4AP.Add(wifiApNode.Get(0)); 
	NetDeviceContainer subnet4APDEV = pointToPointBetweenRouters.Install (subnet4AP);
	address.SetBase ("198.168.45.0", "255.255.255.0");
	Ipv4InterfaceContainer Router4APInterfaces = address.Assign (subnet4APDEV);

	
	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetChannel (channel.Create ());


	WifiHelper wifi;
	wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

	WifiMacHelper mac;
	Ssid ssid = Ssid ("ns-3-ssid");
	mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

	NetDeviceContainer staDevices;
	staDevices = wifi.Install (phy, mac, wifiStaNodes);

	mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

	NetDeviceContainer apDevices;
	apDevices = wifi.Install (phy, mac, wifiApNode.Get(0));

	MobilityHelper mobilityWifi;


	 mobilityWifi.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

    mobilityWifi.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	mobilityWifi.Install (wifiStaNodes);

	mobilityWifi.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobilityWifi.Install (wifiApNode.Get(0));

	address.SetBase ("192.168.1.0", "255.255.255.0");
	address.Assign (staDevices);
	address.Assign (apDevices);

	/* ********************************************************************************************************************************  */
	


	//SIMULATION SIMULATION SIMULATION
	// 1. Communication between Node 15 connected to R1 and Node 15 connected to R2.  
	// Server at Port 9
	UdpEchoServerHelper echoServer (9);
	// Start R1 server
	ApplicationContainer serverApps = echoServer.Install (csmaNodesR1.Get(15));
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (50.0));
	//Start R1 - CSMA Client
	UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (512));

	// Start our application
	UdpEchoServerHelper echoServerR2 (10);
	// Start R2 server
	ApplicationContainer serverAppsR2 = echoServerR2.Install (csmaNodesR2.Get(15));
	serverAppsR2.Start (Seconds (3.0));
	serverAppsR2.Stop (Seconds (53.0));
	//Start R2 - CSMA Client
	UdpEchoClientHelper echoClient2 (csmaInterfacesR2.GetAddress (nCsma), 10);
	echoClient2.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.)));
	echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientAppsR2 = echoClient2.Install (Routers.Get (0));
	ApplicationContainer clientApps = echoClient.Install (Routers.Get (1));

	clientApps.Start (Seconds (2.0));
	clientApps.Stop (Seconds (50.0));
	
	clientAppsR2.Start (Seconds (20.0));
	clientAppsR2.Stop (Seconds (51.0));
/*
	// 2. Communication between Star Node 5 and Node 9 of R2.
	UdpEchoServerHelper echoServerStar (11);
	ApplicationContainer serverAppsStar = echoServerStar.Install (star.GetSpokeNode(5));
	serverAppsStar.Start (Seconds (2.0));
	serverAppsStar.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClientStar (star.GetSpokeIpv4Address(5), 11);
	echoClientStar.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClientStar.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClientStar.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientAppsStar = echoClientStar.Install (csmaNodesR2.Get(9));
	clientAppsStar.Start (Seconds (4.0));
	clientAppsStar.Stop (Seconds (10.0));
	
	
	
	// End of 2.

	// 3. Communication between Wifi node and R1 15 Node
	UdpEchoServerHelper echoServerWiFi (12);
	ApplicationContainer serverAppsWiFi = echoServerWiFi.Install (csmaNodesR1.Get (nCsma));
	serverAppsWiFi.Start (Seconds (4.0));
	serverAppsWiFi.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClientWiFi (csmaInterfaces.GetAddress (nCsma), 12);
	echoClientWiFi.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClientWiFi.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClientWiFi.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientAppsWiFi = echoClientWiFi.Install (wifiStaNodes.Get (nWifi - 2));
	clientAppsWiFi.Start (Seconds (5.0));
	clientAppsWiFi.Stop (Seconds (10.0));
*/
// Create the animation object and configure for specified output
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (Routers);
	mobility.Install (csmaNodesR1);
	mobility.Install (csmaNodesR2);


	star.BoundingBox (112.5, 30, 150, 75);
	AnimationInterface anim (animFile);
	//anim.SetMaxPktsPerTraceFile(1);
	// Routers
	anim.SetConstantPosition(Routers.Get(0), 10,20,0);
	anim.SetConstantPosition(Routers.Get(1), 10,100,0);
	anim.SetConstantPosition(Routers.Get(2), 75,55,0);
	anim.SetConstantPosition(Routers.Get(3), 75,80,0);
	
	//AP
	anim.SetConstantPosition(wifiApNode.Get(0),  0,10,0);
	// CSMA Nodes of Router 1
	anim.SetConstantPosition(csmaNodesR1.Get(1), 10 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(2), 20,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(3), 30 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(4), 40 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(5), 50 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(6), 60 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(7), 70 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(8), 80 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(9), 90 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(10), 100 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(11), 110 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(12), 120 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(13), 130 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(14), 140 ,10,0);
	anim.SetConstantPosition(csmaNodesR1.Get(15), 150 ,10,0);



	anim.SetConstantPosition(csmaNodesR2.Get(1), 10,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(2), 20,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(3), 30,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(4), 40 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(5), 50 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(6), 60 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(7), 70 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(8), 80 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(9), 90 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(10), 100 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(11), 110 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(12), 120 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(13), 130 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(14), 140 ,110,0);
	anim.SetConstantPosition(csmaNodesR2.Get(15), 150 ,110,0);
	pointToPointBetweenRouters.EnablePcapAll ("second");
	// Did not understand.
	csma.EnablePcap ("second", csmaDevicesR2.Get (1), true);
	pointToPointBetweenRouters.EnablePcapAll ("first");
	csma.EnablePcap ("first", csmaDevices.Get (1), true);
	
	
	anim.EnablePacketMetadata (); // Optional
	anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (50)); // Optional
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("dynamic-global-routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (12), routingStream);
    //Simulator::Stop (Seconds (100.0));

	Simulator::Run ();
	std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
	Simulator::Stop (Seconds (10.0));
	Simulator::Destroy ();

	return 0;
}

