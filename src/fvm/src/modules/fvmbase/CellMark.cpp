#include "CellMark.h"
typedef Vector<double, 3> VectorT3;
typedef Array<Vector<double, 3> > VectorT3Array;


int 
inCell(const int cellIndex, 
       const VectorT3& point, 
       const CRConnectivity& faceCells,
       const CRConnectivity& cellFaces,
       const VectorT3Array& faceArea,
       const VectorT3Array& faceCentroid)
{
 
  
  
  int faceNumber=cellFaces.getCount(cellIndex);
  int flag[faceNumber];
  int throwFlag=0;
  int sum=0;
  VectorT3 Af;
 
  //const Array<int>& cellFacesRow = cellFaces.getRow();
  //const Array<int>& cellFacesCol = cellFaces.getCol();

  //const Array<int>& faceCellsRow = faceCells.getRow();
  //const Array<int>& faceCellsCol = faceCells.getCol();
  
  // cout<<"cell index is"<<cellIndex<<endl;
 
  for (int nf=0; nf<faceNumber; nf++){
    const int f=cellFaces(cellIndex, nf);
   
    //first, judge c0 or c1 to define orientation
    const int c0=faceCells(f,0);
    //const int c1=faceCells(f,1);       
    if (cellIndex==c0){
     Af=(-faceArea[f]);
    }
    else {
     Af=(faceArea[f]);
    }  
  
    //calculate product    
    VectorT3 ds=point-faceCentroid[f];
    double product=dot(Af,ds);
    
    if(product > 0.0){ flag[nf]=1;}
    else if (product < 0.0) {flag[nf]=-1;}
    else {
      //cout<<cellIndex<<endl;
      throwFlag = 1;
    }
    sum+=flag[nf];
  }
  //particle is on face or vertex, throw it away
  if (throwFlag == 1){
    return (0);
  }
  //particle is in or out of cell
  else{
    if (sum==faceNumber){
      return (1);   
    }
    else return (-1);
  }
}





void markCell( Mesh& mesh, const int nCells, 
	       const CRConnectivity& cellParticles, const CRConnectivity& cellCells )
{

  //step 1: Mark the cells with solid particles as solid
  //and Mark the cells with no solid particles as fluid

   for(int c=0; c<nCells; c++){
      const int particleCount = cellParticles.getCount(c);
      //if no particle in cell, mark as fluid
      if (particleCount == 0) {
	mesh.setIBTypeForCell(c,Mesh::IBTYPE_FLUID);
      }
      //if has particle in cell, mark as solid
      else { mesh.setIBTypeForCell(c,Mesh::IBTYPE_SOLID); }
    }

   //step2: in solid cells, mark cells with no fluid neighbors as solid
   //and mark cells with at least one fluid neighbors as IB cells

    for (int c=0; c<nCells; c++){
      const int ibType = mesh.getIBTypeForCell(c);
      int flag;
      //search all solid cells
      if(ibType == Mesh::IBTYPE_SOLID){
	flag=1;  //true for solid cells
	const int ncNumber=cellCells.getCount(c);
	for(int nc=0; nc<ncNumber; nc++){
	  const int cellIndex=cellCells(c,nc);
	  if(mesh.getIBTypeForCell(cellIndex)==Mesh::IBTYPE_FLUID){ 
	    flag=0;   
	  }
	}
	//if solid cell has at least one fluid cell neighbor, mark as IBM type
	if(flag==0) mesh.setIBTypeForCell(c,Mesh::IBTYPE_BOUNDARY);
      }
    }
}


void reportCellMark (const Mesh& mesh, const int nCells,
		     const VectorT3Array& cellCentroid,
		     const string fileBase)
{
    
    string fileName=fileBase+"Debug2/CellMark.dat";
    char* file;
    file=&fileName[0];
    FILE *fp=fopen(file,"w");
    
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      fprintf(fp, "%i\t%i\n", c, ibType);
    } 
    fclose(fp);  

    string fileName1 = fileBase+"Debug2/FluidCell.dat";
    string fileName2 = fileBase+"Debug2/IBMCell.dat";
    string fileName3 = fileBase+"Debug2/SolidCell.dat";
    char* file1;
    char* file2;
    char* file3;
    file1=&fileName1[0];
    file2=&fileName2[0];
    file3=&fileName3[0];
    FILE *fp1, *fp2, *fp3;
    fp1=fopen(file1,"w");
    fp2=fopen(file2,"w");
    fp3=fopen(file3,"w");
   
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      if(ibType == Mesh::IBTYPE_FLUID){
	fprintf(fp1, "%i\t%f\t%f\t%f\n", c, cellCentroid[c][0],cellCentroid[c][1],cellCentroid[c][2]);
      }
      else if(ibType==Mesh::IBTYPE_BOUNDARY){
	fprintf(fp2, "%i\t%f\t%f\t%f\n", c, cellCentroid[c][0],cellCentroid[c][1],cellCentroid[c][2]);
      }
      else if(ibType==Mesh::IBTYPE_SOLID){
	fprintf(fp3, "%i\t%f\t%f\t%f\n", c, cellCentroid[c][0],cellCentroid[c][1],cellCentroid[c][2]);
      }
    } 
    fclose(fp1);  
    fclose(fp2);  
    fclose(fp3);  
}


void markIBFaces(Mesh& mesh, const int nCells, 
		 const CRConnectivity& cellCells,
		 const CRConnectivity& cellFaces,
		 const CRConnectivity& faceCells)
{
  //definition of ibFaces: the faces between IB cells and Fluid cells
  //first, count the number of ibFaces
    int ibFaceCount=0;
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      if(ibType==Mesh::IBTYPE_BOUNDARY){
	const int ncNumber=cellCells.getCount(c);
	for(int nc=0; nc<ncNumber; nc++){
	  const int cellIndex=cellCells(c,nc);
	  if(mesh.getIBTypeForCell(cellIndex)==Mesh::IBTYPE_FLUID){ 
	    ibFaceCount++;   
	  }
	}	
      }
    }
    cout<<"ibFaceCount is "<<ibFaceCount<<endl;

      
    //then, allocate an array for ibFace
    mesh.createIBFaceList(ibFaceCount);

    //insert the entries to ibface array
    ibFaceCount=0;
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      if(ibType==Mesh::IBTYPE_BOUNDARY){
	const int faceNumber=cellFaces.getCount(c);
	for(int f=0; f<faceNumber; f++){
	  const int faceIndex=cellFaces(c,f);
	  const int c0 = faceCells(faceIndex,0);
	  const int c1 = faceCells(faceIndex,1);
	  if((c0 == c)&&(mesh.getIBTypeForCell(c1)==Mesh::IBTYPE_FLUID)){
	    mesh.addIBFace(ibFaceCount, faceIndex);
	    ibFaceCount++;
	  }
	  if((c1 == c)&&(mesh.getIBTypeForCell(c0)==Mesh::IBTYPE_FLUID)){
	    mesh.addIBFace(ibFaceCount, faceIndex);
	    ibFaceCount++;
	  }
	}
      }
    }
   
    //initialize storagesite ibFaces
    StorageSite&  ibFaces = mesh.getIBFaces();
    ibFaces.setCount(ibFaceCount);

    /* test ibFaceList
    const Array<int>& ibFaceList = mesh.getIBFaceList();

    for(int i=0; i<ibFaceList.getLength();i++){
      cout<<i<<"   "<<ibFaceList[i]<<endl;
    }
    */

}

const shared_ptr<CRConnectivity> setibFaceParticles 
                          (const Mesh& mesh,
			   const StorageSite& ibFaces, 
			   const Array<int>& ibFaceList,
			   const StorageSite& particles,
			   const CRConnectivity& faceCells, 
			   const CRConnectivity& cellParticles,
			   const Array<int>& particleType)
{

  //CR connectivity cellParticles includes all the particles located in each cell
  //here, we want to create ibFace to Particles in which only the surface particles are counted in
  //surface particles are identified by particle type 1
					       
  shared_ptr<CRConnectivity> ibFaceParticles (new CRConnectivity (ibFaces, particles));
  int maxcount = 0;
  //initCount: new Array for row
  (*ibFaceParticles).initCount();
  const int rowSize = ibFaces.getCount();

  //specify the number of nonzeros for each row
  
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const int C0 = faceCells(faceIndex, 0);
    const int C1 = faceCells(faceIndex, 1);
    if (mesh.getIBTypeForCell(C0) == Mesh::IBTYPE_BOUNDARY){  
      const int nP = cellParticles.getCount(C0);
      if(nP>=maxcount) 
	maxcount=nP;    
      int count=0;
      for(int n=0; n<nP; n++){
	int pID = cellParticles(C0, n);
	if(particleType[pID] == 1){
	  count++;
	  }
      }
      if(count < 2){
	cout<<"Warning!!! Not enough surface particles to interpolate!"<<count<<endl;
      }
      //cout<<"face-cell-particle# "<<faceIndex<<" "<<C0<<" "<<count<<endl;
      
      (*ibFaceParticles).addCount(p, count);      
    }
    else if(mesh.getIBTypeForCell(C1) == Mesh::IBTYPE_BOUNDARY){  
      const int nP = cellParticles.getCount(C1);
      if(nP>=maxcount) 
	maxcount=nP;    
      int count=0;
      for(int n=0; n<nP; n++){
	int pID = cellParticles(C1, n);
	if(particleType[pID] == 1){
	  count++;
	}
      }
      if(count < 2){
	cout<<"Warning!!! Not enough surface particles to interpolate!"<<count<<endl;
      }
      cout<<"face-cell-particle# "<<faceIndex<<" "<<C0<<" "<<count<<endl;
      
      (*ibFaceParticles).addCount(p, count);           
    }
    else cout<<"ibface to particle error!"<<endl;
  }

  cout<<"max count of particles in IB Cells is "<<maxcount<<endl;
  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*ibFaceParticles).finishCount();

  //add in the entries for each row
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const int C0 = faceCells(faceIndex, 0);
    const int C1 = faceCells(faceIndex, 1);
    if (mesh.getIBTypeForCell(C0) == Mesh::IBTYPE_BOUNDARY){   //ib cells
      const int nP = cellParticles.getCount(C0);
      for(int n=0; n<nP; n++){
	int pID=cellParticles(C0,n);
	if(particleType[pID] == 1){
	  (*ibFaceParticles).add(p, pID);
	}
      }
    }
    else if(mesh.getIBTypeForCell(C1) == Mesh::IBTYPE_BOUNDARY){ //ib cells
      const int nP = cellParticles.getCount(C1);
      for(int n=0; n<nP; n++){
	int pID=cellParticles(C1,n);
	if(particleType[pID] == 1){
	  (*ibFaceParticles).add(p, pID);
	}
      }     
    }
     else cout<<"ibface to particle error!"<<endl;
  }
  
  (*ibFaceParticles).finishAdd();
  
  return(ibFaceParticles);
}

const shared_ptr<CRConnectivity> setibFaceCells 
                          (const Mesh& mesh,
			   const Array<int>& ibFaceList,
			   const StorageSite& ibFaces, 
			   const StorageSite& cells,
			   const CRConnectivity& faceCells,
			   Octree& O,
			   const VecD3Array& faceCentroid)
{

  shared_ptr<CRConnectivity> ibFaceCells (new CRConnectivity (ibFaces, cells));
  int maxcount=0;
  //initCount: new Array for row
  (*ibFaceCells).initCount();
  
  const int rowSize = ibFaces.getCount();

  //specify a radius for search
  //const int nCells = cells.getCount();
  const double radius = 0.013;

  //specify the number of nonzeros for each row
 

  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const VecD3 center = faceCentroid[faceIndex];
    int count=0;
    //find the neighbors of center point within a radius
    vector<int> cellIndexList;
    O.getNodes(center, radius, cellIndexList);
    for(int c=0; c< (int)cellIndexList.size(); c++){
      const int cellCandidate = cellIndexList[c];
      if (mesh.getIBTypeForCell(cellCandidate) == Mesh::IBTYPE_FLUID)    
	count++;
    }
    (*ibFaceCells).addCount(p, count);   
    if (count>=maxcount)
      maxcount=count;
  }
    
  cout<<"max Cell neibhbors  "<<maxcount<<endl;
  
  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*ibFaceCells).finishCount();

  //add in the entries for each row
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const VecD3 center = faceCentroid[faceIndex];
    vector<int> cellIndexList;
    O.getNodes(center, radius, cellIndexList);
    for(int c=0; c< (int) cellIndexList.size(); c++){
      const int cellCandidate = cellIndexList[c];
      if (mesh.getIBTypeForCell(cellCandidate) == Mesh::IBTYPE_FLUID)  
	(*ibFaceCells).add(p, cellCandidate);
    }
  }
   
  (*ibFaceCells).finishAdd();
  
  return (ibFaceCells);
}


const  shared_ptr<CRConnectivity> setParticleCells
                          (const StorageSite& rowSite,
			   const StorageSite& colSite, 
			   const Array<int> & connectivity)
{
  const int rowSize = rowSite.getCount();
  //  const int colSize = colSite.getCount();

  shared_ptr<CRConnectivity> rowCol (new CRConnectivity (rowSite, colSite));

  //initCount: new Array for row
  (*rowCol).initCount();

  //specify the number of nonzeros for each row
  //here, each solid point only has connectivity with one cell
  //so for each row, it has only one nonzero
  for(int p=0; p<rowSize; p++){
    int value = connectivity[p];
    if (value != -1)
      (*rowCol).addCount(p, 1);
  }

  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*rowCol).finishCount();

  //add in the entries for each row
  for(int p=0; p<rowSize; p++){
    int value = connectivity[p];
    if (value != -1)
      (*rowCol).add(p, value);
  }

  (*rowCol).finishAdd();
  return(rowCol);

}
