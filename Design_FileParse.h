/***************************************************************************************
* File Name		: Design_FileParse.h
* Author		: Rohit Sreekumar
* Date			: 10/14/2018
****************************************************************************************/

#ifndef DESIGN_FILE_PARSE_H
#define DESIGN_FILE_PARSE_H



/***************************************************************
* User Definitions
****************************************************************/

/*********************** Node Type Names *******************/
#define FDRE		0001
#define LUT6		0010
#define LUT5		0011
#define LUT4		0100
#define LUT3		0101
#define LUT2		0110
#define LUT1		0111
#define CARRY8		1000
#define DSP48E2		1001
#define RAMB36E2	1010
#define BUFGCE		1011
#define IBUF		1100
#define OBUF		1101

/*********************** Node Type sizes *******************/
#define FDRE_AREA		5	
#define LUT6_AREA		7
#define LUT5_AREA		6
#define LUT4_AREA		5
#define LUT3_AREA		4
#define LUT2_AREA		3
#define LUT1_AREA		2
#define CARRY8_AREA		34
#define DSP48E2_AREA	429
#define RAMB36E2_AREA	379
#define BUFGCE_AREA		3
#define IBUF_AREA		2
#define OBUF_AREA		2

#define LARGEST_AREA_CELL 429
/***************************************************************
* Data Structures
****************************************************************/


/***************************************************************
* Enumeration
****************************************************************/
typedef enum
{
	NO_PART = -1,
	A_PART  =  0,
	B_PART  =  1

}NODE_PART_NAME;

typedef enum
{
	UNLOCKED = 0,
	LOCKED   = 1,

}LOCK_STATE;

/***************************************************************
* Structures
****************************************************************/
typedef struct
{
	short nodeTypeName;
	unsigned short area;

}NODE_TYPE;

/***************************************************************
* Class Definitions
****************************************************************/
class NETS;
class NODE;

/***************************************************************
* NODE class
****************************************************************/
class NODE
{
public:

	//Data members
	int nodeName;
	NODE_TYPE nodeType;
	NODE_PART_NAME Node_Partition;
	int nodeGain;
	LOCK_STATE lockedState;
	list<NETS *> connectedNets;
	
	//Constructor
	NODE()
	{
		nodeName = -1;
		nodeType.area = 0;
		nodeType.nodeTypeName = -1;
		Node_Partition = NO_PART;
		nodeGain = 0;
		lockedState = UNLOCKED;
	}

	/* Member Functions */
	FM_RESULT FillClass_NodeDataStructure(int nodeName, short nodeTypeName);
	
	//void RandomizePartition();
	
	void ViewNodeContents();
	
	NODE* FindNode(int nodeName);
	
	FM_RESULT SetNodeGain();


};

/***************************************************************
* NETS class
****************************************************************/
class NETS
{

public:

	//Data members
	string netName;
	unsigned int numConnectedNodes;
	bool criticality;
	int num_Nodes_Part_A;
	int num_Nodes_Part_B;
	list <NODE *> connectedNodes;
	

	//Constructor
	NETS()
	{
		netName = "";
		criticality = false;
		numConnectedNodes = 0;
		num_Nodes_Part_A = 0;
		num_Nodes_Part_B = 0;
	}

	/* Member Functions */
	FM_RESULT FillClass_NetDataStructure(string netName, unsigned int numConnectedNodes);
	
	FM_RESULT Fill_ConnectedNodes(NODE *node_current);
	
	void ViewNetData();
};

/***************************************************************
* Function Declarations
****************************************************************/

/*********************************************************************************
* Function Name: Read_InputFiles
**********************************************************************************/
FM_RESULT Read_InputFiles(FILE *cmd_fp);

/******************************************************************************************
* Function Name: Fill_NodeDataStructure
*******************************************************************************************/
FM_RESULT Fill_NodeDataStructure(int nodeNumber, short nodeTypeNumber, NODE *node_current);

/******************************************************************************************
* Function Name: Read_NodesFile
*******************************************************************************************/
FM_RESULT Read_NodesFile(char *design_node_fileName);

/******************************************************************************************
* Function Name: Fill_NetsDataStructure
*******************************************************************************************/
FM_RESULT Fill_NetsDataStructure(NETS *nets_var, string netName, unsigned int numConnectedNodes, NODE *add_Node, unsigned short flag);

/******************************************************************************************
* Function Name: Read_NetsFile
*******************************************************************************************/
FM_RESULT Read_NetsFile(char *design_nets_fileName);

/******************************************************************************************
* Function Name: FindNodeType
*******************************************************************************************/
short FindNodeType(string nodeTypeName);

/******************************************************************************************
* Function Name: GetNodeArea
*******************************************************************************************/
short GetNodeArea(short nodeTypeName);

/******************************************************************************************
* Function Name: CheckNetCriticality
*******************************************************************************************/
FM_RESULT CheckNetCriticality(NETS *given_Net);

/******************************************************************************************
* Function Name: PlaceNode_In_Partition
*******************************************************************************************/
FM_RESULT PlaceNode_In_Partition(NODE *node_current);

/******************************************************************************************
* Function Name: Check_Adding_To_Cutset
*******************************************************************************************/
FM_RESULT Check_Adding_To_Cutset(NETS *nets_var);

#endif