
#include "Common/CommonCore.h"
#include "Example/HelloGame.h"
#include "Mesh/AssetLoad.h"
#include "Mesh/StaticMesh.h"
using namespace std;


int main()
{
	/*OBJ Mode;
	Mode.Load("Asset/Cube.obj");
	cout << endl;*/
	/*UINT n = Mode.indices.size();
	for (int i = 0; i < n -2; i+=3)
	{
		std::cout << Mode.indices[i]<<" " << Mode.indices[i + 1]<<" " << Mode.indices[i + 2];
		std::cout << std::endl;
	}*/
	//OBJ Mode;
	//Mode.Load("Asset/Cube.obj");

	StaticMesh Mesh;

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
	}

	return 0;
}