#include<iostream>
#define M 1000
using namespace std;

#pragma region Basic definitions
int n = 0; bool matrix[M][M];  // n is the order of the matrix, matrix is the adjacency matrix of the directed graph
struct Point
{
	int in[M];  // points connected to this point, where in[0] stands for the in-degree of this point
	int out[M];  // points connected from this point, where out[0] stands for the out-degree of this point
	Point() { in[0] = 0; out[0] = 0; }  // constructor, initialize in[0] and out[0] to be 0
}points[M + 1];  // points of the directed graph
struct DAGPoint
{
	bool in;  // whether the in-degree of this strongly-connected component is not 0
	int out[M];
	int set[M + 1];  // points of this strongly-connected component in the original graph, where set[0] stands for the number of original points
	DAGPoint() { in = false; out[0] = 0; set[0] = 0; }  // constructor, initialize in, out[0] and set[0] to be 0
}DAGpoints[M + 1];  // points of the converted directed acyclic graph
#pragma endregion
#pragma region Definitions associated with the Tarjan's algorithm
int stack[M + 1] = {}, number[M + 1] = {}, lowlink[M + 1] = {}, index = 0, SCcomponentNumber = 0;
// SCcomponentNumber is the number of strongly-connected components in the graph, the number and lowlink of a point is defined in Tarjan's paper
bool inStack[M + 1] = {};
// whether a point is in the stack
int inComponent[M + 1] = {}, vstack[M + 1] = {}, istack[M + 1] = {};
// inComponent[i] stands for which strongly-connected component (or which DAGPoint) is point i in, vstack is the stack of vertices, 
// istack stands for how many edges from this point have been searched
// int checkDAGmatrix[M][M] = {};
#pragma endregion
#pragma region Definitions associated with the STCorrect algorithm
bool marked[M + 1] = {}, sinkNotFound;
// marked stands for whether this point has been marked, sinkNotFound stands for whether a sink has been found in a search
int source[M + 1] = {}, sink[M + 1] = {}, isolated[M + 1] = {};
// source are points whose in-degree is 0 and out-degree is not, sink are points whose out-degree is 0 and in-degree is not, 
// isolated are points whose in-degree and out-degree are both 0
int W, V, p = 0, w[M / 2 + 1] = {}, v[M / 2 + 1] = {};
// W is used to record the sink in a search, V is the starting point of a search, v[] and w[] are permutations of source[] and sink[] (see Tarjan's paper)
bool inw[M + 1] = {}, inv[M + 1] = {};
// whether a point is in v[] or w[]
#pragma endregion

#pragma region Tarjan's algorithm (recursive version)
void Tarjan(int i)
{
	number[i] = lowlink[i] = ++index;  // assignment of number, initialize lowlink with number, increase index
	inStack[i] = true;  // push into the stack
	stack[++stack[0]] = i;  // push into the stack
	for (int k = 1; k <= points[i].out[0]; k++)
	{
		if (!number[points[i].out[k]])  // some point connected from this point has not been searched
		{
			Tarjan(points[i].out[k]);  // DFS
			lowlink[i] = min(lowlink[i], lowlink[points[i].out[k]]);  // compute lowlink
		}
		else if (inStack[points[i].out[k]])lowlink[i] = min(lowlink[i], number[points[i].out[k]]);  // compute lowlink
	}
	if (number[i] == lowlink[i])  // then all the points in stack above i (inclusive) compose a strongly-connected component
	{
		SCcomponentNumber++;  // increase the number of strongly-connected components
		do
		{
			inStack[stack[stack[0]]] = false;  // pull out of the stack
			DAGpoints[SCcomponentNumber].set[++DAGpoints[SCcomponentNumber].set[0]] = stack[stack[0]];  // compress points in stack into a DAGPoint
			inComponent[stack[stack[0]--]] = SCcomponentNumber;  // record which strongly-connected component a point is in
		} while (stack[stack[0] + 1] != i);  // remove points above i (inclusive) from the stack
	}
}
#pragma endregion
#pragma region Tarjan's algorithm (non-recursive version)
void TarjanNoRecursion()
{
	int vp, ip, wp; bool jExists;  // vp and ip are the vertex and edge on top of their stacks, wp is a temporary variable,
	// jExists stands for whether there is a point unsearched from this point
	while (vstack[0])
	{
		vp = vstack[vstack[0]]; ip = istack[vstack[0]--];  // take the variable on top of stack
		if (ip == 1)  // first time searched
		{
			number[vp] = lowlink[vp] = ++index;  // initialization
			stack[++stack[0]] = vp;  // push into the stack
			inStack[vp] = true;  // push into the stack
		}
		jExists = false;  // initialization
		for (int j = ip; j <= points[vp].out[0]; j++)
		{
			if (!number[points[vp].out[j]])  // some point connected from this point has not been searched
			{
				vstack[++vstack[0]] = vp;
				istack[vstack[0]] = j + 1;  // search from the (j + 1)th edge next time
				vstack[++vstack[0]] = points[vp].out[j];
				istack[vstack[0]] = 1;  // push into the stack
				jExists = true;  // there exists an unsearched point from this point
				break;
			}
			else if (inStack[points[vp].out[j]])
				if (number[points[vp].out[j]] < lowlink[vp])lowlink[vp] = number[points[vp].out[j]];  // compute lowlink
		}
		if (jExists)continue;
		if (number[vp] == lowlink[vp])  // then all the points in stack above vp (inclusive) compose a strongly-connected component
		{
			SCcomponentNumber++;  // increase the number of strongly-connected components
			do
			{
				inStack[stack[stack[0]]] = false;  // pull out of the stack
				DAGpoints[SCcomponentNumber].set[++DAGpoints[SCcomponentNumber].set[0]] = stack[stack[0]];
				// record which strongly-connected component a point is in
				inComponent[stack[stack[0]--]] = SCcomponentNumber;  // remove points above vp (inclusive) from the stack
			} while (stack[stack[0] + 1] != vp);  // remove points above vp (inclusive) from the stack
		}
		if (vstack[0])
		{
			wp = vp; vp = vstack[vstack[0]];
			if (lowlink[wp] < lowlink[vp])lowlink[vp] = lowlink[wp];  // compute lowlink
		}
	}
}
#pragma endregion
#pragma region STCorrect algorithm (recursive version)
void STCorrect(int x)
{
	if (!marked[x])  // x is not marked
	{
		for (int i = 1; i <= sink[0]; i++)
			if (x == sink[i]) { sinkNotFound = false; W = x; break; }  // if x is a sink, then sinkNotFound is false
		marked[x] = true;  // mark x
		for (int i = 1; i <= DAGpoints[x].out[0]; i++)
			if (sinkNotFound)STCorrect(DAGpoints[x].out[i]);  // if sink is not found, keep searching
	}
}
#pragma endregion
#pragma region STCorrect algorithm (non-recursive version)
void STCorrectNoRecursion()
{
	while (stack[0])
	{
		int x = stack[stack[0]]; bool iExists = false;  // take x on top of stack, iExists stands for whether there is a point unsearched from this point
		for (int i = 1; i <= sink[0]; i++)
			if (x == sink[i]) { sinkNotFound = false; W = x; break; }  // if x is a sink, then sinkNotFound is false
		if (sinkNotFound)
			for (int i = 1; i <= DAGpoints[x].out[0]; i++)
				if (!marked[DAGpoints[x].out[i]]) { stack[++stack[0]] = DAGpoints[x].out[i]; iExists = true; break; }  // if sink is not found, keep searching
		if (!iExists) { marked[x] = true; stack[0]--; }  // all points from this point are searched, pull this point out of stack
	}
}
#pragma endregion

int main()
{
#pragma region Input the adjacency matrix and record the graph using points
	char comma;  // read the comma
	cin >> matrix[0][n++];
	while (cin.peek() != '\n')
	{
		cin >> comma >> matrix[0][n++];
		if (matrix[0][n - 1])
		{
			points[1].out[++points[1].out[0]] = n;  // record points connected from point
			points[n].in[++points[n].in[0]] = 1;  // record points connected to point
		}
	}
	for (int i = 1; i < n; i++)
	{
		cin >> matrix[i][0];
		if (matrix[i][0])
		{
			points[i + 1].out[++points[i + 1].out[0]] = 1;  // record points connected from point
			points[1].in[++points[1].in[0]] = i + 1;  // record points connected to point
		}
		for (int j = 1; j < n; j++)
		{
			cin >> comma >> matrix[i][j];
			if (matrix[i][j])
			{
				points[i + 1].out[++points[i + 1].out[0]] = j + 1;  // record points connected from point
				points[j + 1].in[++points[j + 1].in[0]] = i + 1;  // record points connected to point
			}
		}
	}
#pragma endregion
#pragma region Check whether the graph has been recorded correctly
	/*for (int i = 1; i <= n; i++)
	{
		cout << "Point " << i << ":" << endl;
		cout << points[i].in[0] << " in: ";
		for (int j = 1; j <= points[i].in[0]; j++)cout << points[i].in[j] << " ";
		cout << endl << points[i].out[0] << " out: ";
		for (int j = 1; j <= points[i].out[0]; j++)cout << points[i].out[j] << " ";
		cout << endl << endl;
	}*/
#pragma endregion
#pragma region Find all strongly-connected components in graph using Tarjan's algorithm (recursive version)
	/*for (int i = 1; i <= n; i++)
		if (!number[i])
			Tarjan(i);*/
#pragma endregion
#pragma region Find all strongly-connected components in graph using Tarjan's algorithm (non-recursive version)
	for (int k = 1; k <= n; k++)
		if (!number[k])
		{
			vstack[++vstack[0]] = k;
			istack[vstack[0]] = 1;  // initialize the stack
			TarjanNoRecursion();
		}
#pragma endregion
#pragma region Check whether the Tarjan's algorithm has run correctly
	/*cout << "SCcomponentNumber: " << SCcomponentNumber << endl;
	for (int i = 1; i <= SCcomponentNumber; i++)
	{
		cout << "SCcomponent " << i << ": ";
		for (int j = 1; j <= DAGpoints[i].set[0]; j++)cout << DAGpoints[i].set[j] << " ";
		cout << endl;
	}*/
#pragma endregion
#pragma region Compress the graph into a directed acyclic graph (DAG)
	int state[M + 1], tmp;  // state represents whether a point has been recorded in in[] and out[] of DAGpoints
	for (int i = 1; i <= SCcomponentNumber; i++)
	{
		for (int j = 1; j <= DAGpoints[i].set[0]; j++)
			for (int k = 1; k <= points[DAGpoints[i].set[j]].in[0]; k++)
			{
				tmp = inComponent[points[DAGpoints[i].set[j]].in[k]];
				if (tmp != i)
				{
					DAGpoints[i].in = true;  // the in-degree of this strongly-connected component is not 0
					break;
				}
			}
		memset(state, 0, sizeof(state));
		for (int j = 1; j <= DAGpoints[i].set[0]; j++)
			for (int k = 1; k <= points[DAGpoints[i].set[j]].out[0]; k++)
			{
				tmp = inComponent[points[DAGpoints[i].set[j]].out[k]];
				if (!state[tmp] && tmp != i)DAGpoints[i].out[++DAGpoints[i].out[0]] = tmp;  // record out[] of DAGpoints
				state[tmp] = 1;
			}
		if (!DAGpoints[i].in && DAGpoints[i].out[0] == 0)isolated[++isolated[0]] = i;  // recorded isolated points
		else if (!DAGpoints[i].in)source[++source[0]] = i;  // record points whose in-degree is 0 and out-degree is not
		else if (DAGpoints[i].out[0] == 0)sink[++sink[0]] = i;  // record points whose out-degree is 0 and in-degree is not
	}
#pragma endregion
#pragma region Check whether the compression is correct
	// output the adjacency list for programmers to check whether the compression is correct
	/*for (int i = 1; i <= SCcomponentNumber; i++)
	{
		cout << "DAGpoints " << i << " :" << endl << DAGpoints[i].out[0] << " out: ";
		for (int j = 1; j <= DAGpoints[i].out[0]; j++)cout << DAGpoints[i].out[j] << " ";
		cout << endl << DAGpoints[i].set[0] << " set: ";
		for (int j = 1; j <= DAGpoints[i].set[0]; j++)cout << DAGpoints[i].set[j] << " ";
		cout << endl << endl;
	}*/
	// output the adjacency matrix for programmes to check whether the compression is correct
	// uncomment the "checkDAGmatrix" line in region "Definitions associated with the Tarjan's algorithm" before uncomment codes below, vice versa
	/*for (int i = 1; i <= SCcomponentNumber; i++)
		for (int j = 1; j <= DAGpoints[i].out[0]; j++)
			checkDAGmatrix[i][DAGpoints[i].out[j]] = 1;
	for (int i = 1; i <= SCcomponentNumber; i++)
	{
		cout << checkDAGmatrix[i][1];
		for (int j = 2; j <= SCcomponentNumber; j++)cout << ',' << checkDAGmatrix[i][j];
		cout << endl;
	}*/
#pragma endregion
#pragma region Check whether sources, sinks and isolated points are recorded correctly
	/*cout << isolated[0] << " isolated points: ";
	for (int i = 1; i <= isolated[0]; i++)cout << isolated[i] << " ";
	cout << endl << source[0] << " sources: ";
	for (int i = 1; i <= source[0]; i++)cout << source[i] << " ";
	cout << endl << sink[0] << " sinks: ";
	for (int i = 1; i <= sink[0]; i++)cout << sink[i] << " ";
	cout << endl;*/
#pragma endregion
#pragma region Find v[] and w[] satisfied using STCorrect algorithm (recursive version)
	/*for (int i = 1; i <= source[0]; i++)
	{
		V = source[i]; W = 0; sinkNotFound = true;  // the starting point V and ending point W of search
		STCorrect(V);
		if (W) { p++; v[p] = V; w[p] = W; inv[V] = true; inw[W] = true; }  // found a sink
	}
	v[0] = p; w[0] = p;
	for (int i = 1; i <= source[0]; i++)
		if (!inv[source[i]])v[++v[0]] = source[i];  // record the rest of v[]
	for (int i = 1; i <= sink[0]; i++)
		if (!inw[sink[i]])w[++w[0]] = sink[i];*/  // record the rest of w[]
		/*cout << "v[1~p]: ";
		for (int i = 1; i <= p; i++)cout << v[i] << " ";
		cout << endl << "w[1~p]: ";
		for (int i = 1; i <= p; i++)cout << w[i] << " ";
		cout << endl << "v[p+1~s]: ";
		for (int i = p + 1; i <= source[0]; i++)cout << v[i] << " ";
		cout << endl << "w[p+1~t]: ";
		for (int i = p + 1; i <= sink[0]; i++)cout << w[i] << " ";
		cout << endl;*/
#pragma endregion
#pragma region Find v[] and w[] satisfied using STCorrect algorithm (non-recursive version)
	for (int i = 1; i <= source[0]; i++)
	{
		V = source[i]; W = 0; sinkNotFound = true; stack[++stack[0]] = V;  // the starting point V and ending point W of search, push V into stack
		STCorrectNoRecursion();
		if (W) { p++; v[p] = V; w[p] = W; inv[V] = true; inw[W] = true; }  // found a sink
	}
	v[0] = p; w[0] = p;
	for (int i = 1; i <= source[0]; i++)
		if (!inv[source[i]])v[++v[0]] = source[i];  // record the rest of v[]
	for (int i = 1; i <= sink[0]; i++)
		if (!inw[sink[i]])w[++w[0]] = sink[i];  // record the rest of w[]
	/*cout << "v[1~p]: ";
	for (int i = 1; i <= p; i++)cout << v[i] << " ";
	cout << endl << "w[1~p]: ";
	for (int i = 1; i <= p; i++)cout << w[i] << " ";
	cout << endl << "v[p+1~s]: ";
	for (int i = p + 1; i <= source[0]; i++)cout << v[i] << " ";
	cout << endl << "w[p+1~t]: ";
	for (int i = p + 1; i <= sink[0]; i++)cout << w[i] << " ";
	cout << endl;*/
#pragma endregion
#pragma region Augment the directed graph to be strongly-connected with least edges possible
	// int minimumEdgesAdded = 0;
	if (source[0] == sink[0])  // s = t
		if (p == 0 && isolated[0] > 1)  // p = 0, q > 1
		{
			matrix[DAGpoints[isolated[isolated[0]]].set[1] - 1][DAGpoints[isolated[1]].set[1] - 1] = 1;  // add (x(q), x(1))
			// minimumEdgesAdded++;
			for (int i = 1; i < isolated[0]; i++)
			{
				matrix[DAGpoints[isolated[i]].set[1] - 1][DAGpoints[isolated[i + 1]].set[1] - 1] = 1;  // add (x(i), x(i + 1)), 1 <= i < q
				// minimumEdgesAdded++;
			}
		}
		else
		{
			for (int i = 1; i < p; i++)
			{
				matrix[DAGpoints[w[i]].set[1] - 1][DAGpoints[v[i + 1]].set[1] - 1] = 1;  // add (w(i), v(i + 1)), 1 <= i < p
				// minimumEdgesAdded++;
			}
			for (int i = p + 1; i <= source[0]; i++)
			{
				matrix[DAGpoints[w[i]].set[1] - 1][DAGpoints[v[i]].set[1] - 1] = 1;  // add (w(i), v(i)), p + 1 <= i <= s
				// minimumEdgesAdded++;
			}
			if (isolated[0] == 0)  // p > 0, q = 0
			{
				matrix[DAGpoints[w[p]].set[1] - 1][DAGpoints[v[1]].set[1] - 1] = 1;  // add (w(p), v(1))
				// minimumEdgesAdded++;
			}
			else if (p > 0)  // p > 0, q > 0
			{
				matrix[DAGpoints[w[p]].set[1] - 1][DAGpoints[isolated[1]].set[1] - 1] = 1;  // add (w(p), x(1))
				// minimumEdgesAdded++;
				for (int i = 1; i < isolated[0]; i++)
				{
					matrix[DAGpoints[isolated[i]].set[1] - 1][DAGpoints[isolated[i + 1]].set[1] - 1] = 1;  // add (x(i), x(i + 1)), 1 <= i < q
					// minimumEdgesAdded++;
				}
				matrix[DAGpoints[isolated[isolated[0]]].set[1] - 1][DAGpoints[v[1]].set[1] - 1] = 1;  // add (x(q), v(1))
				// minimumEdgesAdded++;
			}
		}
	else if (source[0] < sink[0])  // s < t
	{
		for (int i = 1; i < p; i++)
		{
			matrix[DAGpoints[w[i]].set[1] - 1][DAGpoints[v[i + 1]].set[1] - 1] = 1;  // add (w(i), v(i + 1)), 1 <= i < p
			// minimumEdgesAdded++;
		}
		for (int i = p + 1; i <= source[0]; i++)
		{
			matrix[DAGpoints[w[i]].set[1] - 1][DAGpoints[v[i]].set[1] - 1] = 1;  // add (w(i), v(i)), p + 1 <= i <= s
			// minimumEdgesAdded++;
		}
		matrix[DAGpoints[w[p]].set[1] - 1][DAGpoints[w[source[0] + 1]].set[1] - 1] = 1;  // add (w(p), w(s + 1))
		// minimumEdgesAdded++;
		for (int i = source[0] + 1; i < sink[0]; i++)
		{
			matrix[DAGpoints[w[i]].set[1] - 1][DAGpoints[w[i + 1]].set[1] - 1] = 1;  // add (w(i), w(i + 1)), s + 1 <= i < t
			// minimumEdgesAdded++;
		}
		if (isolated[0] == 0)  // q = 0
		{
			matrix[DAGpoints[w[sink[0]]].set[1] - 1][DAGpoints[v[1]].set[1] - 1] = 1;  // add (w(t), v(1))
			// minimumEdgesAdded++;
		}
		else  // q > 0
		{
			matrix[DAGpoints[w[sink[0]]].set[1] - 1][DAGpoints[isolated[1]].set[1] - 1] = 1;  // add (w(t), x(1))
			// minimumEdgesAdded++;
			for (int i = 1; i < isolated[0]; i++)
			{
				matrix[DAGpoints[isolated[i]].set[1] - 1][DAGpoints[isolated[i + 1]].set[1] - 1] = 1;  // add (x(i), x(i + 1)), 1 <= i < q
				// minimumEdgesAdded++;
			}
			matrix[DAGpoints[isolated[isolated[0]]].set[1] - 1][DAGpoints[v[1]].set[1] - 1] = 1;  // add (x(q), v(1))
			// minimumEdgesAdded++;
		}
	}
	else  // s > t
	{
		for (int i = 1; i < p; i++)
		{
			matrix[DAGpoints[w[i + 1]].set[1] - 1][DAGpoints[v[i]].set[1] - 1] = 1;  // add (w(i + 1), v(i)), 1 <= i < p
			// minimumEdgesAdded++;
		}
		for (int i = p + 1; i <= sink[0]; i++)
		{
			matrix[DAGpoints[w[i]].set[1] - 1][DAGpoints[v[i]].set[1] - 1] = 1;  // add (w(i), v(i)), p + 1 <= i <= t
			// minimumEdgesAdded++;
		}
		matrix[DAGpoints[v[sink[0] + 1]].set[1] - 1][DAGpoints[v[p]].set[1] - 1] = 1;  // add (v(t + 1), v(p))
		// minimumEdgesAdded++;
		for (int i = sink[0] + 1; i < source[0]; i++)
		{
			matrix[DAGpoints[v[i + 1]].set[1] - 1][DAGpoints[v[i]].set[1] - 1] = 1;  // add (v(i + 1), v(i)), t + 1 <= i < s
			// minimumEdgesAdded++;
		}
		if (isolated[0] == 0)  // q = 0
		{
			matrix[DAGpoints[w[1]].set[1] - 1][DAGpoints[v[source[0]]].set[1] - 1] = 1;  // add (w(1), v(s))
			// minimumEdgesAdded++;
		}
		else  // q > 0
		{
			matrix[DAGpoints[isolated[1]].set[1] - 1][DAGpoints[v[source[0]]].set[1] - 1] = 1;  // add (x(1), v(s))
			// minimumEdgesAdded++;
			for (int i = 1; i < isolated[0]; i++)
			{
				matrix[DAGpoints[isolated[i + 1]].set[1] - 1][DAGpoints[isolated[i]].set[1] - 1] = 1;  // add (x(i + 1), x(i)), 1 <= i < q
				// minimumEdgesAdded++;
			}
			matrix[DAGpoints[w[1]].set[1] - 1][DAGpoints[isolated[isolated[0]]].set[1] - 1] = 1;  // add (w(1), x(q))
			// minimumEdgesAdded++;
		}
	}
	// cout << source[0] << " sources" << endl;
	// cout << sink[0] << " sinks" << endl;
	// cout << isolated[0] << " isolated points" << endl;
	// cout << "Edges added: " << minimumEdgesAdded << endl;
	for (int i = 0; i < n; i++)
	{
		cout << matrix[i][0];
		for (int j = 1; j < n; j++)cout << "," << matrix[i][j];
		cout << endl;
	}
#pragma endregion
	return 0;
}