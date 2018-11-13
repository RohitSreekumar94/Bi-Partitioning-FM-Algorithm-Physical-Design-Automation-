#include "Headers.h"
#include "Misc_Func.h"
#include "Design_FileParse.h"
#include "FM_Part_Algorithm.h"


/***************************************************************
* External Variables
****************************************************************/

//Get the node and net objects from design parse file
extern NETS *nets;
extern NODE *node;
extern unsigned int numNodes;
extern unsigned int numNets;

//Total circuit area (nodes)
extern unsigned int Total_area;

/* Total of area of nodes in partition A */
extern unsigned int A_PART_AREA;

/* Total of area of nodes in partition B */
extern unsigned int B_PART_AREA;

extern unsigned int CUT_SET;

/***************************************************************
* Global Variables
****************************************************************/

// A single bucket to hold the gain and a vector all nodes with that gain
map <int, vector<NODE*> > MAP_GAIN_NODE;

unsigned int MIN_CUT_SET = 0;

unsigned int lockedNodes = 0;
unsigned int UPPER_LIMIT_AREA_CONSTRAINT = 0;
unsigned int LOWER_LIMIT_AREA_CONSTRAINT = 0;

// This is a map of nodes with their corresponding partition. This information is saved during each min cut set
map<int, NODE_PART_NAME> savedState;





/******************************************************************************************
* Class : NODE
* Class Member Function Name: SetNodeGain
* Description: Member Fn to fill node gain values
*******************************************************************************************/
FM_RESULT NODE::SetNodeGain()
{
	NODE_PART_NAME F_Block = this->Node_Partition;
	NODE_PART_NAME T_Block = NO_PART;

	int *F_n = NULL;
	int *T_n = NULL;


	list<NETS *>::iterator iter_nets;
	list<NODE *>::iterator iter_nodes;

	if (F_Block == A_PART)
	{
		T_Block = B_PART;
	}
	else if (F_Block == B_PART)
	{
		T_Block = A_PART;
	}
	else
	{
		printf("\nError SetNodeGain():: Invalid node partition");
		return FM_ERROR;
	}

	
	//Iterating through each of the nets connected to this node
	for (iter_nets = this->connectedNets.begin(); iter_nets != this->connectedNets.end(); ++iter_nets)
	{
		if (F_Block == A_PART)
		{
			F_n = &(*iter_nets)->num_Nodes_Part_A;
			T_n = &(*iter_nets)->num_Nodes_Part_B;
		}
		else if (F_Block == B_PART)
		{
			F_n = &(*iter_nets)->num_Nodes_Part_B;
			T_n = &(*iter_nets)->num_Nodes_Part_A;
		}

		//Calculate FS_node
		if ((*F_n) == 1)
		{
			this->nodeGain++;
		}

		//Calculate TE_node
		if ((*T_n) == 0)
		{
			this->nodeGain--;
		}
	}

	//Updating the map
	MAP_GAIN_NODE[this->nodeGain].push_back(this);


	return FM_SUCCESS;
}


/***************************************************************************
* Function Name: InitializeNodeGain
* Description: Function to calcualte node gain for each node
****************************************************************************/
void InitializeNodeGain()
{
	//Setting gain for each node with the given initial random partition
	for (unsigned int i = 0; i < numNodes; i++)
	{
		//Reset lock status and node gain
		node[i].lockedState = UNLOCKED;
		node[i].nodeGain = 0;

		//Set proper node gain
		node[i].SetNodeGain();

	}
}

/***************************************************************************
* Function Name: FindMaxElement_MAP
* Description: Function to get max gain node
****************************************************************************/
NODE * FindMaxElement_MAP()
{
	map < int, vector<NODE*> > ::reverse_iterator iter;
	int				check			= 0;
	int				flag			= 0;
	unsigned int	i				= 0;
	int				swapped			= false;
	NODE			*swappedNode	= NULL;
	vector<NODE *>	*node_max;

	//Get the highest gain node from the gain bucket
	for (iter = MAP_GAIN_NODE.rbegin(); iter != MAP_GAIN_NODE.rend(); iter++)
	{
		//Get highest gain node from the map
		node_max = &iter->second;

		//Loop through the vector to get an unlocked node
		for (i = 0; i < node_max->size(); i++)
		{
			if ((*node_max)[i]->lockedState == LOCKED)
			{
				continue;
			}
			else
			{
				swapped = SwapPartition_and_Check_Area((*node_max)[i]);
				
				//If the condition is true that means node has been moved and locked
				if (1 == swapped)
				{
					swappedNode = (*node_max)[i];
					if ((*node_max)[i]->lockedState == LOCKED)
					{
						node_max->erase(node_max->begin() + i);
					}
					break;
				}	
			}
		}

		if (1 == swapped)
		{
			break;
		}
	}

	return swappedNode;
	
}


/***************************************************************************
* Function Name: SwapPartition
* Description: Function to swap the partition and check for area constraint
****************************************************************************/
int SwapPartition_and_Check_Area(NODE *node)
{
	NODE_PART_NAME	org_Node_part	= NO_PART;
	NODE_PART_NAME	new_Node_part	= NO_PART;
	bool			part_OK			= false;
	unsigned int	prev_A_area		= A_PART_AREA;
	unsigned int	prev_B_area		= B_PART_AREA;

	//Store original partition
	org_Node_part = node->Node_Partition;

	if (A_PART == org_Node_part)
	{
		new_Node_part = B_PART;
	}
	else
	{
		new_Node_part = A_PART;
	}

	//Find the new area post swapping
	Update_Area_For_Each_Partition(node,new_Node_part);

	//Check for area constraint validity
	part_OK = CheckAreaContraint();
	if (false == part_OK)
	{
		node->Node_Partition = org_Node_part;
		A_PART_AREA = prev_A_area;
		B_PART_AREA = prev_B_area;
		return 0;
	}
	else
	{
		//node->Node_Partition = new_Node_part;
		node->lockedState = LOCKED;
		return 1;
	}
}



/***************************************************************************
* Function Name: Recalcualte_NodeGain
* Description: Function to recalculate all the gain for connected nets only
****************************************************************************/
void Recalculate_NodeGain(NODE* node_current)
{
	unsigned int	FS_node = 0;
	unsigned int	TE_node = 0;
	int				flag = 0;
	int				prevGain = 0;
	int				newGain = 0;
	int				keyNotFound = 0;
	unsigned int	i = 0;
	int             v_flag = 0;
	NODE_PART_NAME	newPart = node_current->Node_Partition;

	int *F_n = NULL;
	int *T_n = NULL;


	list<NETS *>::iterator iter_nets;
	list<NETS *>::iterator iter_nets_node;
	list<NODE *>::iterator iter_nodes;
	list<NODE *>::iterator iter_nodes_nets;

	vector<NODE*> *nodeList = NULL;

	//The "from" block of the base cell prior to move
	NODE_PART_NAME F_BaseCell = node_current->Node_Partition;
	
	//The "to" block of the base cell
	NODE_PART_NAME T_BaseCell = NO_PART;

	if (A_PART == F_BaseCell)
	{
		T_BaseCell = B_PART;

		//Complimenting the base cell partition
		node_current->Node_Partition = B_PART;
	}
	else if (B_PART == F_BaseCell)
	{
		T_BaseCell = A_PART;

		//Complimenting the base cell partition
		node_current->Node_Partition = A_PART;
	}
	else
	{
		printf("\nError Recalculate_NodeGain() :: Invalid node part");
		return;
	}


	// This is prior to swapping
	//Iterating through each of the nets connected to the moved node
	for (iter_nets = node_current->connectedNets.begin(); iter_nets != node_current->connectedNets.end(); ++iter_nets)
	{
		//Check critical nets beofre move
		//Find T(n)
		if (T_BaseCell == A_PART)
		{
			T_n = &(*iter_nets)->num_Nodes_Part_A;
			F_n = &(*iter_nets)->num_Nodes_Part_B;
		}
		else if (T_BaseCell == B_PART)
		{
			T_n = &(*iter_nets)->num_Nodes_Part_B;
			F_n = &(*iter_nets)->num_Nodes_Part_A;
		}

		if ((*T_n) == 0)
		{

			//Iterating through each of the connnected node in this particular net and incrementing the gains of all unlocked nodes
			for (iter_nodes = (*iter_nets)->connectedNodes.begin(); iter_nodes != (*iter_nets)->connectedNodes.end(); ++iter_nodes)
			{
				//Clearing everything in the vector
				if (NULL != nodeList)
				{
					nodeList = NULL;
				}

				if ((*iter_nodes)->lockedState == LOCKED)
				{
					continue;
				}

				prevGain = (*iter_nodes)->nodeGain;
				(*iter_nodes)->nodeGain += 1;
				newGain = (*iter_nodes)->nodeGain;

				//Update MAP
				nodeList = &MAP_GAIN_NODE[prevGain];

				for (i = 0; i < (*nodeList).size(); i++)
				{
					if ((*iter_nodes)->nodeName == (*nodeList)[i]->nodeName)
					{
						v_flag = 1;
						break;
					}
				}

				if (1 == v_flag)
				{
					//Erase prev gain node
					(*nodeList).erase((*nodeList).begin() + i);

					//Add the node with new gain to the map
					MAP_GAIN_NODE[newGain].push_back((*iter_nodes));
				}
				v_flag = 0;
			}
		}

		
		else if ((*T_n) == 1)
		{
			for (iter_nodes = (*iter_nets)->connectedNodes.begin(); iter_nodes != (*iter_nets)->connectedNodes.end(); ++iter_nodes)
			{
				//Clearing everything in the vector
				if (NULL != nodeList)
				{
					nodeList = NULL;
				}

				// Decrement the gain of the only "to" node on the net
				if (T_BaseCell == (*iter_nodes)->Node_Partition)
				{
					if ((*iter_nodes)->lockedState == LOCKED)
					{
						break;
					}

					prevGain = (*iter_nodes)->nodeGain;
					(*iter_nodes)->nodeGain -= 1;
					newGain = (*iter_nodes)->nodeGain;

					//Update MAP
					nodeList = &MAP_GAIN_NODE[prevGain];

					for (i = 0; i < (*nodeList).size(); i++)
					{
						if ((*iter_nodes)->nodeName == (*nodeList)[i]->nodeName)
						{
							v_flag = 1;
							break;
						}
					}

					if (1 == v_flag)
					{
						//Erase prev gain node
						(*nodeList).erase((*nodeList).begin() + i);

						//Add the node with new gain to the map
						MAP_GAIN_NODE[newGain].push_back((*iter_nodes));
					}
					v_flag = 0;
				}
			}
		}

		// Changed the net distribution to reflect the move
		(*F_n)--;
		(*T_n)++;

		// Check critical nets after the move
		if ((*F_n) == 0)
		{
			//Iterating through each of the connnected node in this particular net - this is the neighbouring nodes
			for (iter_nodes = (*iter_nets)->connectedNodes.begin(); iter_nodes != (*iter_nets)->connectedNodes.end(); ++iter_nodes)
			{
				//Clearing everything in the vector
				if (NULL != nodeList)
				{
					nodeList = NULL;
				}

				if ((*iter_nodes)->lockedState == LOCKED)
				{
					continue;
				}
				prevGain = (*iter_nodes)->nodeGain;
				(*iter_nodes)->nodeGain -= 1;
				newGain = (*iter_nodes)->nodeGain;

				//Update MAP
				nodeList = &MAP_GAIN_NODE[prevGain];

				for (i = 0; i < (*nodeList).size(); i++)
				{
					if ((*iter_nodes)->nodeName == (*nodeList)[i]->nodeName)
					{
						v_flag = 1;
						break;
					}
				}

				if (1 == v_flag)
				{
					//Erase prev gain node
					(*nodeList).erase((*nodeList).begin() + i);

					//Add the node with new gain to the map
					MAP_GAIN_NODE[newGain].push_back((*iter_nodes));
				}
				v_flag = 0;
			}
		}

		else if ((*F_n) == 1)
		{
			
			for (iter_nodes = (*iter_nets)->connectedNodes.begin(); iter_nodes != (*iter_nets)->connectedNodes.end(); ++iter_nodes)
			{
				//Clearing everything in the vector
				if (NULL != nodeList)
				{
					nodeList = NULL;
				}

				// This is the to partition
				if (F_BaseCell == (*iter_nodes)->Node_Partition)
				{
					if ((*iter_nodes)->lockedState == LOCKED)
					{
						break;
					}

					prevGain = (*iter_nodes)->nodeGain;
					(*iter_nodes)->nodeGain += 1;
					newGain = (*iter_nodes)->nodeGain;

					//Update MAP
					nodeList = &MAP_GAIN_NODE[prevGain];

					for (i = 0; i < (*nodeList).size(); i++)
					{
						if ((*iter_nodes)->nodeName == (*nodeList)[i]->nodeName)
						{
							v_flag = 1;
							break;
						}
					}

					if (1 == v_flag)
					{
						//Erase prev gain node
						(*nodeList).erase((*nodeList).begin() + i);

						//Add the node with new gain to the map
						MAP_GAIN_NODE[newGain].push_back((*iter_nodes));
					}
					v_flag = 0;
				}
			}
		}
	}
}



/*********************************************************************************
* Function Name: Update_Area_For_Each_Partition
* Description: Function to set area of each partition
**********************************************************************************/
FM_RESULT Update_Area_For_Each_Partition(NODE *movedNode, NODE_PART_NAME new_Node_part)
{
	if (A_PART == new_Node_part)
	{
		A_PART_AREA += movedNode->nodeType.area;
		B_PART_AREA -= movedNode->nodeType.area;
	}
	else if (B_PART == new_Node_part)
	{
		B_PART_AREA += movedNode->nodeType.area;
		A_PART_AREA -= movedNode->nodeType.area;
	}
	else
	{
		printf("\nError Update_Area_For_Each_Partition() :: Invalid node partition");
		return FM_ERROR;
	}

	return FM_SUCCESS;

}

/*********************************************************************************
* Function Name: CheckAreaContraint
* Description: Function to check wheather area constraint is met or not
**********************************************************************************/
bool CheckAreaContraint()
{
	if ((LOWER_LIMIT_AREA_CONSTRAINT < A_PART_AREA  && A_PART_AREA < UPPER_LIMIT_AREA_CONSTRAINT) && 
		(LOWER_LIMIT_AREA_CONSTRAINT < B_PART_AREA && B_PART_AREA < UPPER_LIMIT_AREA_CONSTRAINT))
	{
		return true;
	}
	else
	{
		return false;
	}
}


/***************************************************************************
* Function Name: Save_CurrentData_State
* Description: Function to save the state of data structures
****************************************************************************/
void Save_CurrentData_State(int min_cut, NETS * savedNets, NODE *savedNode)
{
	//Clearing the map of all previous values
	savedState.clear();

	/*for (unsigned int i = 0; i < numNodes; i++)
	{
		savedState.insert(pair<int, NODE_PART_NAME>(node[i].nodeName, node[i].Node_Partition));
	}*/

	if (NULL != savedNode)
	{
		memset(savedNode, 0, sizeof(NODE)*numNodes);
	}

	//Copy entire node datastructure to savedNode
	memcpy(savedNode, node, sizeof(NODE)*numNodes);

	if (NULL != savedNets)
	{
		memset(savedNets, 0, sizeof(NETS)*numNets);
	}

	//Copy entire nets datastructure to savedNet
	memcpy(savedNets, nets, sizeof(NETS)*numNets);
}


/***************************************************************************
* Function Name: SetupAreaConstraint
* Description: Function to set the limits for the area constraints
****************************************************************************/
void SetupAreaConstraint()
{
	double area_1 = 0;
	double area_2 = 0;

	area_1 = AREA_RATIO * Total_area;
	area_2 = (1 - AREA_RATIO) * Total_area;

	if (area_1 > area_2)
	{
		UPPER_LIMIT_AREA_CONSTRAINT = (unsigned int)round(area_1);
		LOWER_LIMIT_AREA_CONSTRAINT = (unsigned int)round(area_2);
	}
	else
	{
		UPPER_LIMIT_AREA_CONSTRAINT = (unsigned int)round(area_2);
		LOWER_LIMIT_AREA_CONSTRAINT = (unsigned int)round(area_1);
	}

}

/***************************************************************************
* Function Name: FM_Alogrithm_Core
* Description: Function to perform FM Algorithm
****************************************************************************/
FM_RESULT FM_Algorithm_Core()
{
	NODE		 *max_node				= NULL;
	int			 i						= 0;
	NODE		 *tempNode				= NULL;
	float		 ratioCut				= 0;
	float		 percentChangeCutset	= 0;
	unsigned int INITIAL_CUTSET			= 0;
	NETS		 *savedNets				= NULL;
	NODE		 *savedNode				= NULL;
	
	//Setting up the area constraint
	SetupAreaConstraint();

	//Allocate area for saved net
	savedNets = new NETS[numNets];

	//Allocate area for saved net
	savedNode = new NODE[numNodes];

#ifdef DEBUG_LOG
	printf("\nTotal Area: %d", Total_area);
	printf("\nArea Upper Limit: %d", UPPER_LIMIT_AREA_CONSTRAINT);
	printf("\nArea Lower Limit: %d", LOWER_LIMIT_AREA_CONSTRAINT);
#endif	

	//Loop for passes
	for (int j = 1; j <= NUM_PASSES; j++)
	{
		MIN_CUT_SET = 0;

		//Initialize node gain  
		InitializeNodeGain();

		//Set inital cut set as minimum
		MIN_CUT_SET = CUT_SET;
		INITIAL_CUTSET = CUT_SET;

		printf("\nPass: %d Initial Cutset: %d", j, INITIAL_CUTSET);

		//Loop for iterations
		while (1)
		{

			//Get node with max gain
			max_node = FindMaxElement_MAP();
			if (NULL == max_node)
			{
				//No more swaps can be made - stop the algorithm
				break;
			}

			//Update cut set when a change has been made

			CUT_SET -= max_node->nodeGain;

			//Recalculation of node gains for the nieghbouring nodes
			Recalculate_NodeGain(max_node);

			//If current cut set is less than minimum cutset then set min cutset to the new value
			if (CUT_SET < MIN_CUT_SET)
			{
				MIN_CUT_SET = CUT_SET;

				//Save state every time a new min cut set is obtained
				Save_CurrentData_State(MIN_CUT_SET, savedNets, savedNode);
			}
		}

		
		printf("\nPass: %d Minimum Cutset: %d", j, MIN_CUT_SET);

		//Calculate ratio cut
		ratioCut = (float)MIN_CUT_SET / INITIAL_CUTSET;
		printf("\nRatio Cut: %f", ratioCut);

		//Calculate percent change in cutset
		percentChangeCutset = (1 - ratioCut) * 100;
		printf("\nPercent imporvement in Cutset: %f", percentChangeCutset);

#ifdef DEBUG_LOG
		printf("\nArea A: %d", A_PART_AREA);
		printf("\nArea B: %d", B_PART_AREA);
#endif
		//Reloading the saved state
		/*for (unsigned int i = 0; i < numNodes; i++)
		{
			if (NULL != node)
			{
				node[i].Node_Partition = savedState[node[i].nodeName];
			}
		}*/

		//Resetting node data structure
		memset(node, 0, sizeof(NODE)*numNodes);

		//Reloading savedNodes
		memcpy(node, savedNode, sizeof(NODE)*numNodes);

		//Resetting nets data structure
		memset(nets, 0, sizeof(NETS)*numNets);

		//Reloading savedNets
		memcpy(nets, savedNets, sizeof(NETS)*numNets);

		CUT_SET = 0;

		//Calculate new cutset
		for (unsigned int i = 0; i < numNets; i++)
		{
			Check_Adding_To_Cutset(&nets[i]);
		}

		MAP_GAIN_NODE.clear();
	}

	return FM_SUCCESS;
}
