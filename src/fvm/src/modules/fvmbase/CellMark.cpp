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
    
    if(product >= 0.0) flag[nf]=1;
    else if (product < 0.0) flag[nf]=-1;
    sum+=flag[nf];
  }
  if (sum==faceNumber){
    return (1);   
  }
  else return (-1);
}


const  shared_ptr<CRConnectivity> getSolidCells(const int nMPM, const int nCells, 
				const Array<int> & MPM_PointstoCells )
{
  
  StorageSite solid(nMPM,0);            //rowSite
  StorageSite cells(nCells,0);          //colSite

  shared_ptr<CRConnectivity> solidCells (new CRConnectivity (solid, cells));

  //initCount: new Array for row
  (*solidCells).initCount();

  //specify the number of nonzeros for each row
  //here, each solid point only has connectivity with one cell
  //so for each row, it has only one nonzero
  for(int p=0; p<nMPM; p++){
    (*solidCells).addCount(p, 1);
  }

  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*solidCells).finishCount();

  //add in the entries for each row
  for(int p=0; p<nMPM; p++){
    int value=MPM_PointstoCells[p];
    (*solidCells).add(p, value);
  }

  (*solidCells).finishAdd();
  return(solidCells);

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
	mesh.setIBTypeForCell(c,0);
      }
      //if has particle in cell, mark as solid
      else { mesh.setIBTypeForCell(c,2); }
    }

   //step2: in solid cells, mark cells with no fluid neighbors as solid
   //and mark cells with at least one fluid neighbors as IB cells

    for (int c=0; c<nCells; c++){
      const int ibType = mesh.getIBTypeForCell(c);
      int flag;
      //search all solid cells
      if(ibType==2){
	flag=1;  //true for solid cells
	const int ncNumber=cellCells.getCount(c);
	for(int nc=0; nc<ncNumber; nc++){
	  const int cellIndex=cellCells(c,nc);
	  if(mesh.getIBTypeForCell(cellIndex)==0){  //has fluid cell neighbor
	    flag=0;   
	  }
	}
	//if solid cell has at least one fluid cell neighbor, mark as IBM type
	if(flag==0) mesh.setIBTypeForCell(c,1);
      }
    }
}


void reportCellMark (const Mesh& mesh, const int nCells,
		     const VectorT3Array& cellCentroid,
		     const string fileBase)
{
    
    string fileName=fileBase+"CellMark.dat";
    char* file;
    file=&fileName[0];
    FILE *fp=fopen(file,"w");
    
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      fprintf(fp, "%i\t%i\n", c, ibType);
    } 
    fclose(fp);  

    string fileName1 = fileBase+"FluidCell.dat";
    string fileName2 = fileBase+"SolidCell.dat";
    string fileName3 = fileBase+"IBMCell.dat";
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
      if(ibType==0){
	fprintf(fp1, "%i\t%f\t%f\t%f\n", c, cellCentroid[c][0],cellCentroid[c][1],cellCentroid[c][2]);
      }
      else if(ibType==1){
	fprintf(fp3, "%i\t%f\t%f\t%f\n", c, cellCentroid[c][0],cellCentroid[c][1],cellCentroid[c][2]);
      }
      else{
	fprintf(fp2, "%i\t%f\t%f\t%f\n", c, cellCentroid[c][0],cellCentroid[c][1],cellCentroid[c][2]);
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
  //first, count the number of ibFaces
    int ibFaceCount=0;
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      if(ibType==1){
	const int ncNumber=cellCells.getCount(c);
	for(int nc=0; nc<ncNumber; nc++){
	  const int cellIndex=cellCells(c,nc);
	  if(mesh.getIBTypeForCell(cellIndex)==0){  //has fluid cell neighbor
	    ibFaceCount++;   
	  }
	}	
      }
    }
    //cout<<"ibFaceCount is "<<ibFaceCount<<endl;
    //then, allocate an array for ibFace
    mesh.createIBFaceList(ibFaceCount);

    //insert the entries for ibface array
    ibFaceCount=0;
    for(int c=0; c<nCells; c++){
      int ibType = mesh.getIBTypeForCell(c);
      if(ibType==1){
	const int faceNumber=cellFaces.getCount(c);
	for(int f=0; f<faceNumber; f++){
	  const int faceIndex=cellFaces(c,f);
	  const int c0 = faceCells(faceIndex,0);
	  const int c1 = faceCells(faceIndex,1);
	  if((c0 == c)&&(mesh.getIBTypeForCell(c1)==0)){
	    mesh.addIBFace(ibFaceCount, faceIndex);
	    ibFaceCount++;
	  }
	  if((c1 == c)&&(mesh.getIBTypeForCell(c0)==0)){
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
			   const CRConnectivity& cellParticles)
{
					       
  shared_ptr<CRConnectivity> ibFaceParticles (new CRConnectivity (ibFaces, particles));

  //initCount: new Array for row
  (*ibFaceParticles).initCount();
  const int rowSize = ibFaces.getCount();

  //specify the number of nonzeros for each row
  
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const int C0 = faceCells(faceIndex, 0);
    const int C1 = faceCells(faceIndex, 1);
    if (mesh.getIBTypeForCell(C0) == 1){
      const int nP = cellParticles.getCount(C0);
     (*ibFaceParticles).addCount(p, nP);
    }
    else if(mesh.getIBTypeForCell(C1) == 1){
      const int nP = cellParticles.getCount(C1);
     (*ibFaceParticles).addCount(p, nP);
    }
  }

  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*ibFaceParticles).finishCount();

  //add in the entries for each row
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const int C0 = faceCells(faceIndex, 0);
    const int C1 = faceCells(faceIndex, 1);
    if (mesh.getIBTypeForCell(C0) == 1){
      const int nP = cellParticles.getCount(C0);
      for(int n=0; n<nP; n++){
	int value=cellParticles(C0,n);
	(*ibFaceParticles).add(p, value);
      }
    }
    else if(mesh.getIBTypeForCell(C1) == 1){
      const int nP = cellParticles.getCount(C1);
      for(int n=0; n<nP; n++){
	int value=cellParticles(C1,n);
	(*ibFaceParticles).add(p, value);
      }     
    }
  }
  
  (*ibFaceParticles).finishAdd();
  
  return(ibFaceParticles);
}

const shared_ptr<CRConnectivity> setibFaceCells 
                          (const Mesh& mesh,
			   const Array<int>& ibFaceList,
			   const StorageSite& ibFaces, 
			   const StorageSite& cells,
			   const CRConnectivity& faceCells)
{

  shared_ptr<CRConnectivity> ibFaceCells (new CRConnectivity (ibFaces, cells));

  //initCount: new Array for row
  (*ibFaceCells).initCount();
  
  const int rowSize = ibFaces.getCount();

  //specify the number of nonzeros for each row
  
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const int nP=1;
    (*ibFaceCells).addCount(p, nP);   
  }

  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*ibFaceCells).finishCount();

  //add in the entries for each row
  for(int p=0; p<rowSize; p++){
    const int faceIndex = ibFaceList [p];
    const int C0 = faceCells(faceIndex, 0);
    const int C1 = faceCells(faceIndex, 1);
    if (mesh.getIBTypeForCell(C0) == 1){
      int value=C1;
      (*ibFaceCells).add(p, value);
    }
    else if(mesh.getIBTypeForCell(C1) == 1){
       int value=C0;
      (*ibFaceCells).add(p, value);
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
  const int colSize = colSite.getCount();

  shared_ptr<CRConnectivity> rowCol (new CRConnectivity (rowSite, colSite));

  //initCount: new Array for row
  (*rowCol).initCount();

  //specify the number of nonzeros for each row
  //here, each solid point only has connectivity with one cell
  //so for each row, it has only one nonzero
  for(int p=0; p<rowSize; p++){
    (*rowCol).addCount(p, 1);
  }

  //finishCount: allocate col array and reset row array
  //ready to get the entries for nonzeros
  (*rowCol).finishCount();

  //add in the entries for each row
  for(int p=0; p<rowSize; p++){
    int value = connectivity[p];
    (*rowCol).add(p, value);
  }

  (*rowCol).finishAdd();
  return(rowCol);

}
