/***************************************************************************************
* File Name		: Design_FileParse.cpp
* Description	: This file reads all the input files and stores the netlist data 
			      into data structures.
****************************************************************************************/

#include "Headers.h"
#include "Misc_Func.h"
#include "Design_FileParse.h"
#include "FM_Part_Algorithm.h"

/****************************************************************************************
* Global Variables
****************************************************************************************/

/*numNodes: Global variable storing the total number of nodes in the given netlist*/
unsigned int numNodes = 0;

/*numNets: Global variable storing the total number of nets in the given netlist*/
unsigned int numNets  = 0;

/*A_Part_Area_Percent: Global variable to hold the current % of nodes in partition A */
float A_Part_Area_Percent = 0;

/*B_Part_Area_Percent: Global variable to hold the current % of nodes in partition B */
float B_Part_Area_Percent = 0;

/*nets: Data structure holding all nets*/
NETS *nets;

/*node: Data structure holding all nodes*/
NODE *node;

/* MAP to quicky find a node object given the node name*/
map<int,NODE*> dict_nodeList;

int rand_g_flag = 0;

/* Variable holding total area of circuit */
unsigned int Total_area = 0;

/* Total of area of nodes in partition A */
unsigned int A_PART_AREA = 0;

/* Total of area of nodes in partition B */
unsigned int B_PART_AREA = 0;

unsigned int CUT_SET = 0;;





/******************************************************************************************
* Class : NODE
* Class Member Function Name: FindNode
* Description: Member Fn to find the node given the node name
*******************************************************************************************/
NODE* NODE :: FindNode(int nodeName)
{
	if (this->nodeName == nodeName)
	{
		return this;
	}
	else
	{
		return NULL;
	}
}

/******************************************************************************************
* Class : NODE
* Class Member Function Name: ViewNodeContents
* Description: Member Fn to view class data
*******************************************************************************************/
void NODE::ViewNodeContents()
{
	printf("NodeName: %d\n", this->nodeName);
	printf("Node Partition: %d\n", this->Node_Partition);
	printf("Node Type: %d\n", this->nodeType.nodeTypeName);
}

/******************************************************************************************
* Class : NODE
* Class Member Function Name: FillClass_NodeDataStructure
* Description: Member Fn to fill class member variables
*******************************************************************************************/
FM_RESULT NODE::FillClass_NodeDataStructure(int nodeName, short nodeTypeName)
{
	short area = 0;

	
	this->nodeName = nodeName;
	this->nodeType.nodeTypeName = nodeTypeName;

	//Function to get area of node
	area = GetNodeArea(nodeTypeName);
	if (-1 == area)
	{
		printf("\nError FillClass_NodeDataStructure():: Member Fn : Invalid node type area");
		return FM_ERROR;
	}

	this->nodeType.area = area;
	

	return FM_SUCCESS;

}


/******************************************************************************************
* Function Name: FindNodeType
* Description: Function to find the nodetype identifier
*******************************************************************************************/
short FindNodeType(string nodeTypeName)
{
	short nodeType_Identifier = -1;
	
	if ("FDRE" == nodeTypeName)
		nodeType_Identifier = FDRE;

	else if ("LUT6" == nodeTypeName)
		nodeType_Identifier = LUT6;

	else if ("LUT5" == nodeTypeName)
		nodeType_Identifier = LUT5;

	else if ("LUT4" == nodeTypeName)
		nodeType_Identifier = LUT4;

	else if ("LUT3" == nodeTypeName)
		nodeType_Identifier = LUT3;

	else if ("LUT2" == nodeTypeName)
		nodeType_Identifier = LUT2;

	else if ("LUT1" == nodeTypeName)
		nodeType_Identifier = LUT1;

	else if ("CARRY8" == nodeTypeName)
		nodeType_Identifier = CARRY8;

	else if ("DSP48E2" == nodeTypeName)
		nodeType_Identifier = DSP48E2;

	else if ("RAMB36E2" == nodeTypeName)
		nodeType_Identifier = RAMB36E2;

	else if ("BUFGCE" == nodeTypeName)
		nodeType_Identifier = BUFGCE;

	else if ("IBUF" == nodeTypeName)
		nodeType_Identifier = IBUF;

	else if ("OBUF" == nodeTypeName)
		nodeType_Identifier = OBUF;

	else
		return -1;


	return nodeType_Identifier;
}

/******************************************************************************************
* Function Name: GetNodeArea
* Description: Function to get node area from node type
*******************************************************************************************/
short GetNodeArea(short nodeTypeName)
{
	short nodeType_Identifier = 0;

	if (FDRE == nodeTypeName)
		nodeType_Identifier = FDRE_AREA;

	else if (LUT6 == nodeTypeName)
		nodeType_Identifier = LUT6_AREA;

	else if (LUT5 == nodeTypeName)
		nodeType_Identifier = LUT5_AREA;

	else if (LUT4 == nodeTypeName)
		nodeType_Identifier = LUT4_AREA;

	else if (LUT3 == nodeTypeName)
		nodeType_Identifier = LUT3_AREA;

	else if (LUT2 == nodeTypeName)
		nodeType_Identifier = LUT2_AREA;

	else if (LUT1 == nodeTypeName)
		nodeType_Identifier = LUT1_AREA;

	else if (CARRY8 == nodeTypeName)
		nodeType_Identifier = CARRY8_AREA;

	else if (DSP48E2 == nodeTypeName)
		nodeType_Identifier = DSP48E2_AREA;

	else if (RAMB36E2 == nodeTypeName)
		nodeType_Identifier = RAMB36E2_AREA;

	else if (BUFGCE == nodeTypeName)
		nodeType_Identifier = BUFGCE_AREA;

	else if (IBUF == nodeTypeName)
		nodeType_Identifier = IBUF_AREA;

	else if (OBUF == nodeTypeName)
		nodeType_Identifier = OBUF_AREA;

	else
		return -1;


	return nodeType_Identifier;
}

/******************************************************************************************
* Function Name: PlaceNode_In_Partition
* Description: Function to place a node in a partition
*******************************************************************************************/
FM_RESULT PlaceNode_In_Partition(NODE *node_current)
{
	int randomPart = rand() & 1; //Randomly choosing either a 0 or a 1

	// If both partition areas are equal randomly place a node in one of the partitions
	if (A_PART_AREA == B_PART_AREA)
	{
		//Set node to a random partition
		node_current->Node_Partition = (NODE_PART_NAME)randomPart;

		if (0 == randomPart)
		{
			A_PART_AREA += node_current->nodeType.area;
		}
		else if (1 == randomPart)
		{
			B_PART_AREA += node_current->nodeType.area;
		}
		else
		{
			printf("\nError PlaceNode_In_Partition():: Invalid random node partition");
			return FM_ERROR;
		}
	}

	// If the area of the one the partition is larger try to balance the partition areas by placing a node in 
	// the other partition.
	else if (A_PART_AREA < B_PART_AREA)
	{
		node_current->Node_Partition = A_PART;
		A_PART_AREA += node_current->nodeType.area;
	}
	// If the area of the one the partition is larger try to balance the partition areas by placing a node in 
	// the other partition.
	else
	{
		node_current->Node_Partition = B_PART;
		B_PART_AREA += node_current->nodeType.area;
	}

	return FM_SUCCESS;
	
}

/******************************************************************************************
* Function Name: Fill_NodeDataStructure
* Description: Function fill node data structure from data read from file
*******************************************************************************************/
FM_RESULT Fill_NodeDataStructure(int nodeNumber, short nodeTypeNumber, NODE *node_current)
{
	FM_RESULT result = FM_ERROR;
	
	result = node_current->FillClass_NodeDataStructure(nodeNumber, nodeTypeNumber);
	if (FM_SUCCESS != result)
	{
		printf("\nError Fill_NodeDataStructure() :: FillClass_NodeDataStructure() member function failed");
		return FM_ERROR;
	}

	
	//Create map map for quickly searching a node
	dict_nodeList.insert(pair<int,NODE*>(nodeNumber, node_current));
	
	//Fill total area
	Total_area += node_current->nodeType.area;
	 
	//Randomly place the node in a partition
	PlaceNode_In_Partition(node_current);

	return result;

}

/******************************************************************************************
* Function Name: Read_NodesFile
* Description: Function to read Nodes file and store data in corresponding data structures
*******************************************************************************************/
FM_RESULT Read_NodesFile(char *design_node_fileName)
{
	FILE			*node_fp		= NULL;
	unsigned int	numLines		= 0;
	char			line[100]	    = { 0 };
	char			resetLine[100]  = { 0 };
	char			inst_Type[32]	= { 0 };
	string			inst_Type_str   = "";
	int				nodeNumber		= -1;
	short			nodeTypeNumber  = -1;
	FM_RESULT		result			= FM_ERROR;
	int				index			= 0;
	int				approxNumNodes = 0;

	if (NULL == design_node_fileName)
	{
		printf("\nError Read_NodesFile() :: Invalid node file name");
		return FM_ERROR;
	}

	//Open file in read mode
	node_fp = fopen(design_node_fileName, "r");
	if (NULL == node_fp)
	{
		printf("\nError Read_NodesFile() :: Invalid node file pointer");
		return FM_ERROR;
	}

	//Get approx number of nodes 
	approxNumNodes = GetSizeofFile(node_fp) / 10;

	//Allocate memory for nodes
	node = new NODE[approxNumNodes];

	//Loop to get each line from the file
	while (fgets(line, 100, node_fp))
	{
		//Keep count of number of nodes
		numNodes++;
	
		//get the node name and node type from the string
		sscanf(line, "inst_%d %s", &nodeNumber, inst_Type);
		
		//Copy node type to string data type
		inst_Type_str = inst_Type;
		
		//Function to find the numerical node type
		nodeTypeNumber = FindNodeType(inst_Type_str);
		if (-1 == nodeTypeNumber)
		{
			printf("\nError Read_NodesFile():: Invalid Node type");
			return FM_ERROR;
		}
		
		//Fill the node data structure with the data read from the file
		result = Fill_NodeDataStructure(nodeNumber, nodeTypeNumber,&node[index]);
		if (FM_ERROR == result)
		{
			printf("\nError Read_NodesFile():: Fill_NodeDataStructure()  failed");
			return FM_ERROR;
		}

		//Reset line 
		memcpy(line, resetLine, 100);

		index++;
	}


	//Close file
	fclose(node_fp);


	return FM_SUCCESS;

}

/******************************************************************************************
* Class : NETS
* Class Member Function Name: FillClass_NetDataStructure
* Description: Member to fill class member variables
*******************************************************************************************/
FM_RESULT NETS::FillClass_NetDataStructure(string netName, unsigned int numConnectedNodes)
{
	this->netName = netName;
	this->numConnectedNodes = numConnectedNodes;

	return FM_SUCCESS;
}

/******************************************************************************************
* Class : NETS
* Class Member Function Name: FillClass_NetDataStructure
* Description: Member Fn to fill class member variables
*******************************************************************************************/
FM_RESULT NETS::Fill_ConnectedNodes(NODE *node_current)
{
	this->connectedNodes.push_back(node_current);

	//Keep count of node partition
	if (A_PART == node_current->Node_Partition)
	{
		this->num_Nodes_Part_A++;
	}
	else if (B_PART == node_current->Node_Partition)
	{
		this->num_Nodes_Part_B++;
	}
	else
	{
		printf("\nError Fill_ConnectedNodes():: Invalid node partition");
		return FM_ERROR;
	}

	return FM_SUCCESS;
}

/******************************************************************************************
* Class : NETS
* Class Member Function Name: ViewNetData
* Description: Member Fn to view data in the net class
*******************************************************************************************/
void NETS::ViewNetData()
{
	list<NODE *>::iterator iter;

	cout << "\nNet Name: " << this->netName.c_str() << "\nNum of Connected Nodes: " << this->numConnectedNodes ;
	cout << "\nConnected Nodes: \n";

	for (iter = this->connectedNodes.begin(); iter != this->connectedNodes.end(); ++iter)
	{
		(*iter)->ViewNodeContents();
	}
}

/******************************************************************************************
* Function Name: Fill_NetsDataStructure
* Description: Function fill nets data structure from data read from file
*******************************************************************************************/
FM_RESULT Fill_NetsDataStructure(NETS *nets_var,string netName, unsigned int numConnectedNodes,NODE *add_Node,unsigned short flag)
{
	FM_RESULT	result = FM_ERROR;
	
	if (0 == flag)
	{
		result = nets_var->FillClass_NetDataStructure(netName, numConnectedNodes);
		if (FM_SUCCESS != result)
		{
			printf("\nError Fill_NetsDataStructure() :: FillClass_NetDataStructure() member function failed");
			return FM_ERROR;
		}
	}
	else
	{
		if (NULL != add_Node)
		{
			result = nets_var->Fill_ConnectedNodes(add_Node);
			if (FM_SUCCESS != result)
			{
				printf("\nError Fill_NetsDataStructure() :: Fill_ConnectedNodes() member function failed");
				return FM_ERROR;
			}

			if (NULL != nets_var)
			{
				//Fill the connected nets
				add_Node->connectedNets.push_back(nets_var);
			}
		}
		else
		{
			printf("\nError Fill_NetsDataStructure() :: Trying to add invalid node to nets linkedlist");
			return FM_ERROR;
		}
	}

	
	return result;

}

/******************************************************************************************
* Function Name: CheckNetCriticality
* Description: Function to check wheather the net is critical or not
*******************************************************************************************/
FM_RESULT CheckNetCriticality(NETS *given_Net)
{
	list <NODE *>:: iterator iter;
	unsigned int num_Part_A = 0;
	unsigned int num_Part_B = 0;


	for (iter = given_Net->connectedNodes.begin() ; iter != given_Net->connectedNodes.end() ; ++iter)
	{
		if (A_PART == (*iter)->Node_Partition)
			num_Part_A++;

		else if (B_PART == (*iter)->Node_Partition)
			num_Part_B++;

		else
		{
			printf("\nError CheckNetCriticality():: Invalid node partition");
			return FM_ERROR;
		}
	}

	//If the number nodes in any partition is 0 or 1 then that net is critical
	if (0 == num_Part_A || 1 == num_Part_A || 0 == num_Part_B || 1 == num_Part_B)
		given_Net->criticality = true;

	return FM_SUCCESS;
}

/******************************************************************************************
* Function Name: Check_Adding_To_Cutset
* Description: Function to check wheather the given net adds to the cutset
*******************************************************************************************/
FM_RESULT Check_Adding_To_Cutset(NETS *nets_var)
{
	if (NULL != nets_var)
	{
		if (nets_var->num_Nodes_Part_A > 0 && nets_var->num_Nodes_Part_B > 0)
		{
			CUT_SET++;
		}
	}
	else
	{
		printf("\nError Check_Adding_To_Cutset() :: Invalid net");
		return FM_ERROR;
	}

	return FM_SUCCESS;
}

/******************************************************************************************
* Function Name: Read_NetsFile
* Description: Function to read Nets file and store data in corresponding data structures
*******************************************************************************************/
FM_RESULT Read_NetsFile(char *design_nets_fileName)
{
	FILE			*nets_fp		= NULL;
	FM_RESULT		result			= FM_ERROR;
	char			line[100]		= { 0 };
	char			resetLine[100]	= { 0 };
	char			netLine[128]	= { 0 };
	string			netLine_str		= "";
	char			netName[64]		= { 0 };
	int				numConnNodes	=  0 ;
	int				netNumber		= 0;
	int				nodeNumber		= 0;
	unsigned int	approxNumNets	= 0;
	int				index			= 0;
	unsigned int	approxSize_Net  = 0;
	map<int, NODE*> ::iterator iter;

	if (NULL == design_nets_fileName)
	{
		printf("\nError Read_NetsFile() :: Invalid net file name");
		return FM_ERROR;
	}

	//Open nets file
	nets_fp = fopen(design_nets_fileName, "r");
	if (NULL == nets_fp)
	{
		printf("\nError Read_NetsFile() :: Invalid net file pointer");
		return FM_ERROR;
	}

	//Get approx num of nets - If not enough memory error hits, decrease the value by which the size of file is divided with
	approxNumNets = GetSizeofFile(nets_fp)/30;

	//Storing the size in a global variable for future reference
	approxSize_Net = approxNumNets;

	//Allocate memory for NETS data structure
	nets = new NETS[approxNumNets];

	while (fgets(line, 100, nets_fp))
	{
		//Get first word from the line
		sscanf(line, "%s ", netLine);

		netLine_str = netLine;
		
		//Check for memory 
		if (numNets > approxNumNets)
		{
			printf("\nError Read_NetsFile():: Not enough memory to store all nets. Increase nets data structure allocated area");
			return FM_ERROR;
		}

		if ("net" == netLine_str)
		{
			//Keep count of number of nets
			numNets++;

			//Get net name and num of connected nodes
			sscanf(line, "%s %s %d", netLine, netName, &numConnNodes);

			//Fill net name and details into data structure
			result = Fill_NetsDataStructure(&nets[index], netName,numConnNodes,NULL,0);
			if (FM_ERROR == result)
			{
				printf("\nError Read_NetsFile():: Fill_NetsDataStructure() function failed");
				return FM_ERROR;
			}

		}
		else if ("endnet" == netLine_str)
		{
			result = CheckNetCriticality(&nets[index]);
			if (FM_ERROR == result)
			{
				printf("\nError Read_NetsFile():: CheckNetCriticality() Function Failed");
				return result;
			}

			//Determine wheather this net contributes to the cutset or not
			result = Check_Adding_To_Cutset(&nets[index]);
			if (FM_ERROR == result)
			{
				printf("\nError Read_NetsFile():: Check_Adding_To_Cutset() Function Failed");
				return result;
			}

			index++;
		}
		else
		{
			//Get net name and num of connected nodes
			sscanf(netLine, "inst_%d ", &nodeNumber);

			//Find node object from map 
			iter = dict_nodeList.find(nodeNumber);
			if (iter == dict_nodeList.end())
			{
				printf("\nError Read_NetsFile() :: Cannot find given node: %d .Invalid node type name.", nodeNumber);
				return FM_ERROR;
			}

			//Fill net name and details into data structure
			result = Fill_NetsDataStructure(&nets[index], "", 0,iter->second,1);
			if (FM_ERROR == result)
			{
				printf("\nError Read_NetsFile():: Fill_NetsDataStructure() function failed");
				return FM_ERROR;
			}
		}
	}

	//Close file
	fclose(nets_fp);


	return result;
}


/*********************************************************************************
* Function Name: Read_InputFiles
* Description: Function to read input files - design.aux, design.nodes,design.nets 
**********************************************************************************/
FM_RESULT Read_InputFiles(FILE *cmd_fp)
{
	
	unsigned int	fileSize					= 0;
	string			delimBuf					= "";
	string			subStr						= "";
	char			*buffer						= NULL;
	int				i							= 0;
	string			token						= "";
	char			design_nodes_fileName[32]   = { 0 };
	char			design_nets_fileName[32]    = { 0 };
	string			tempStr						= "";
	FM_RESULT		result						= FM_ERROR;
	char			strTemp[32]					= { 0 };

/*****************************************************************
* Reading 'design.aux' file and reading the sub file names from it
******************************************************************/

	//Function to get file size
	fileSize = GetSizeofFile(cmd_fp);

	//Dynamically allocate memory for file read buffer
	buffer = (char *)calloc(fileSize, sizeof(char));

	//Read file contents
	fread(buffer, sizeof(char), fileSize, cmd_fp);

	//Close file
	fclose(cmd_fp);

	// Iterate until the end of first line
	for (i = 0; buffer[i] != '\n'; i++);

	//Store 2nd line into the given variable
	delimBuf = buffer + i;

	//Get the file names from the string
	sscanf(delimBuf.c_str(), "%s : %s %s", strTemp, design_nodes_fileName, design_nets_fileName);

	
	//Freeing the buffer
	free(buffer);

/***********************  design.aux file analysis done ****************************/


/*****************************************************************
* Reading sub files - design.nodes & design.nets
******************************************************************/

	//Reading design.nodes file
	result = Read_NodesFile(design_nodes_fileName);
	if (FM_ERROR == result)
	{
		printf("\nError Read_InputFiles() :: Read_NodesFile() Failed");
		return result;
	}
	
	//Reading design.nets file
	result = Read_NetsFile(design_nets_fileName);
	if (FM_ERROR == result)
	{
		printf("\nError Read_InputFiles() :: Read_NetsFile() Failed");
		return result;
	}


	return result;
}