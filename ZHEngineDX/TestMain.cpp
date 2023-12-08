
#include "Common/CommonCore.h"
#include "Example/HelloGame.h"
#include "Mesh/AssetLoad.h"
#include "Mesh/StaticMesh.h"
#include <algorithm>
using namespace std;

struct V3
{
	int x;
	int y;
	int z;
};

struct condition
{
	bool operator()(const V3& A, const V3& B)
	{
		if (A.z > B.z)
		{
			return true;
		}
		// Z相同则按Y从小到大排序
		else if (A.y < B.y)
		{
			return true;
		}
		// Y相同则按X从小到大排序
		else if (A.x < B.x)
		{
			return true;
		}
		return false;
	}
};

int main()
{

	/*StaticMesh Mesh;

	MeshAssetLoad(Mesh, "Asset/Mode.uasset");
	for (auto& A : Mesh.vertices)
	{
		cout << A.position.x << " " << A.position.y << " " << A.position.z << endl;
		cout << A.normal.x << " " << A.normal.y << " " << A.normal.z << endl;
		cout << A.tangent.x << " " << A.tangent.y << " " << A.tangent.z << endl;
		cout << A.texcoord.x << " " << A.texcoord.y <<  endl;
		cout << endl;
	}
	int n = Mesh.indices.size();
	for (int i=0;i<n-2;i+=3)
	{
		cout << Mesh.indices[i] << " " << Mesh.indices[i + 1] << " " << Mesh.indices[i + 2] << endl;
	}*/
	// { {1, 2, 1}, { 2,5,7 }, { 6,1,3 }, { 4,1,2 }, { 5,5,5 }}
	

	



	system("pause");
	return 0;
}