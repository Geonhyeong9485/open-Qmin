#include "multirankQTensorLatticeModel.h"
#include "multirankQTensorLatticeModel.cuh"
/*! \file multirankQTensorLatticeModel.cpp" */

multirankQTensorLatticeModel::multirankQTensorLatticeModel(int lx, int ly, int lz, bool _xHalo, bool _yHalo, bool _zHalo, bool _useGPU)
    : qTensorLatticeModel(lx,ly,lz,_useGPU), xHalo(_xHalo), yHalo(_yHalo), zHalo(_zHalo)
    {
    int Lx = lx;
    int Ly = ly;
    int Lz = lz;
    latticeSites.x = Lx;
    latticeSites.y = Ly;
    latticeSites.z = Lz;
    determineBufferLayout();

    if(xHalo)
        Lx +=2;
    if(yHalo)
        Ly +=2;
    if(zHalo)
        Lz +=2;

    totalSites = Lx*Ly*Lz;
    expandedLatticeSites.x = Lx;
    expandedLatticeSites.y = Ly;
    expandedLatticeSites.z = Lz;
    expandedLatticeIndex = Index3D(expandedLatticeSites);
    positions.resize(totalSites);
    types.resize(totalSites);
    forces.resize(totalSites);
    velocities.resize(totalSites);

    printf("%i vs %i\n",totalSites,N+transferStartStopIndexes[25].y);
    }

/*!
Meant to be used with idx in (transferStartStopIndexes[directionType].x to ".y)
*/
void multirankQTensorLatticeModel::getBufferInt3FromIndex(int idx, int3 &pos,int directionType, bool sending)
    {
    int startIdx = transferStartStopIndexes[directionType].x;
    int index = idx - startIdx;
    switch(directionType)
        {
        case 0: 
            pos.z = index / latticeSites.y; pos.y = index % latticeSites.y;
            pos.x = sending ? 0 : -1;
            break;
        case 1: 
            pos.z = index / latticeSites.y; pos.y = index % latticeSites.y;
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            break;
        case 2: 
            pos.z = index / latticeSites.x; pos.x = index % latticeSites.x;
            pos.y = sending ? 0 : -1;
            break;
        case 3: 
            pos.z = index / latticeSites.x; pos.x = index % latticeSites.x;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            break;
        case 4: 
            pos.y = index / latticeSites.x; pos.x = index % latticeSites.x;
            pos.z = sending ? 0 : -1;
            break;
        case 5: 
            pos.y = index / latticeSites.x; pos.x = index % latticeSites.x;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            break;
        //edges
        case 6:
            pos.x = sending ? 0 : -1;
            pos.y = sending ? 0 : -1;
            pos.z = index;
            break;
        case 7:
            pos.x = sending ? 0 : -1;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = index;
            break;
        case 8:
            pos.x = sending ? 0 : -1;
            pos.z = sending ? 0 : -1;
            pos.y = index;
            break;
        case 9:
            pos.x = sending ? 0 : -1;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            pos.y = index;
            break;
        case 10:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.y = sending ? 0 : -1;
            pos.z = index;
            break;
        case 11:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = index;
            break;
        case 12:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.z = sending ? 0 : -1;
            pos.y = index;
            break;
        case 13:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            pos.y = index;
            break;
        case 14:
            pos.y = sending ? 0 : -1;
            pos.z = sending ? 0 : -1;
            pos.x = index;
            break;
        case 15:
            pos.y = sending ? 0 : -1;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            pos.x = index;
            break;
        case 16:
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = sending ? 0 : -1;
            pos.x = index;
            break;
        case 17:
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            pos.x = index;
            break;
        //corners
        case 18:
            pos.x = sending ? 0 : -1;
            pos.y = sending ? 0 : -1;
            pos.z = sending ? 0 : -1;
            break;
        case 19:
            pos.x = sending ? 0 : -1;
            pos.y = sending ? 0 : -1;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            break;
        case 20:
            pos.x = sending ? 0 : -1;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = sending ? 0 : -1;
            break;
        case 21:
            pos.x = sending ? 0 : -1;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            break;
        case 22:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.y = sending ? 0 : -1;
            pos.z = sending ? 0 : -1;
            break;
        case 23:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.y = sending ? 0 : -1;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            break;
        case 24:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = sending ? 0 : -1;
            break;
        case 25:
            pos.x = sending ? latticeSites.x-1 : latticeSites.x;
            pos.y = sending ? latticeSites.y-1 : latticeSites.y;
            pos.z = sending ? latticeSites.z-1 : latticeSites.z;
            break;
        }
    }


/*!
Given the buffer layout below, determine the data array index of a given position
*/
int multirankQTensorLatticeModel::positionToIndex(int3 &pos)
    {
    if(pos.x <0 && !xHalo)
        pos.x = latticeSites.x-1;
    if(pos.x ==latticeSites.x && !xHalo)
        pos.x = 0;
    if(pos.y <0 && !yHalo)
        pos.y = latticeSites.y-1;
    if(pos.y ==latticeSites.y && !yHalo)
        pos.y = 0;
    if(pos.z <0 && !zHalo)
        pos.z = latticeSites.z-1;
    if(pos.z ==latticeSites.z && !zHalo)
        pos.z = 0;

    if(pos.x < latticeSites.x && pos.y < latticeSites.y && pos.z < latticeSites.z && pos.x >=0 && pos.y >= 0 && pos.z >= 0)
        return latticeIndex(pos);

    int base = N;
    //0: x = -1 face
    base = N + transferStartStopIndexes[0].x;
    if(pos.x == -1 && ordered(0, pos.y, latticeSites.y-1) && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.y+latticeSites.y*pos.z;
    //1: x = max face +1
    base = N + transferStartStopIndexes[1].x;
    if(pos.x == latticeSites.x && ordered(0, pos.y, latticeSites.y-1) && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.y+latticeSites.y*pos.z;
    //2: y = -1
    base = N + transferStartStopIndexes[2].x;
    if(pos.y == -1 && ordered(0, pos.x, latticeSites.x-1) && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.x+latticeSites.x*pos.z;
    //3: y = max face
    base = N + transferStartStopIndexes[3].x;
    if(pos.y == latticeSites.y && ordered(0, pos.x, latticeSites.x-1) && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.x+latticeSites.x*pos.z;
    //4: z = -1
    base = N + transferStartStopIndexes[4].x;
    if(pos.z == -1 && ordered(0, pos.x, latticeSites.x-1) && ordered(0,pos.y,latticeSites.y-1))
        return base + pos.x+latticeSites.x*pos.y;
    //5: z = max face+1
    base = N + transferStartStopIndexes[5].x;
    if(pos.z == latticeSites.z && ordered(0, pos.x, latticeSites.x-1) && ordered(0,pos.y,latticeSites.y-1))
        return base + pos.x+latticeSites.x*pos.y;

    //6: x = -1, y = -1  edge
    base = N + transferStartStopIndexes[6].x;
    if(pos.x==-1 && pos.y==-1 && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.z;
    //7: x = -1, y = max  edge
    base = N + transferStartStopIndexes[7].x;
    if(pos.x==-1 && pos.y==latticeSites.y && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.z;
    //8: x = -1, z = -1  edge
    base = N + transferStartStopIndexes[8].x;
    if(pos.x==-1 && pos.z==-1 && ordered(0,pos.y,latticeSites.y-1))
        return base + pos.y;
    //9: x = -1, z = max  edge
    base = N + transferStartStopIndexes[9].x;
    if(pos.x==-1 && pos.z==latticeSites.z  && ordered(0,pos.y,latticeSites.y-1))
        return base + pos.y;
    //10: x = max, y = -1  edge
    base = N + transferStartStopIndexes[10].x;
    if(pos.x== latticeSites.x && pos.y==-1 && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.z;
    //11: x = max, y = max  edge
    base = N + transferStartStopIndexes[11].x;
    if(pos.x== latticeSites.x && pos.y==latticeSites.y && ordered(0,pos.z,latticeSites.z-1))
        return base + pos.z;
    //12: x = max, z = -1  edge
    base = N + transferStartStopIndexes[12].x;
    if(pos.x==latticeSites.x && pos.z==-1 && ordered(0,pos.y,latticeSites.y-1))
        return base + pos.y;
    //13: x = max, z = max  edge
    base = N + transferStartStopIndexes[13].x;
    if(pos.x==latticeSites.x && pos.z==latticeSites.z && ordered(0,pos.y,latticeSites.y-1))
        return base + pos.y;
    //14: y = -1, z = -1  edge
    base = N + transferStartStopIndexes[14].x;
    if(pos.y==-1 && pos.z==-1 && ordered(0,pos.x,latticeSites.x-1))
        return base + pos.x;
    //15: y = -1, z = max  edge
    base = N + transferStartStopIndexes[15].x;
    if(pos.y==-1 && pos.z==latticeSites.z && ordered(0,pos.x,latticeSites.x-1))
        return base + pos.x;
    //16: y = max, z = -1  edge
    base = N + transferStartStopIndexes[16].x;
    if(pos.y==latticeSites.y && pos.z==-1 && ordered(0,pos.x,latticeSites.x-1))
        return base + pos.x;
    //17: y = max, z = max  edge
    base = N + transferStartStopIndexes[17].x;
    if(pos.y==latticeSites.y && pos.z==latticeSites.z && ordered(0,pos.x,latticeSites.x-1))
        return base + pos.x;

    //18: x = -1, y = -1, z=-1 corner
    base = N + transferStartStopIndexes[18].x;
    if(pos.x == -1)
        {
        if(pos.y == -1)
            {
            if (pos.z == -1)
                return base;
            else if (pos.z == latticeSites.z)
                return base + 1;
            }
        else if(pos.y == latticeSites.y)
            {
            if (pos.z == -1)
                return base+2;
            else if (pos.z == latticeSites.z)
                return base+3;
            }
        }
    else if (pos.x == latticeSites.x)
        {
        if(pos.y == -1)
            {
            if (pos.z == -1)
                return base+4;
            else if (pos.z == latticeSites.z)
                return base+5;
            }
        else if(pos.y == latticeSites.y)
            {
            if (pos.z == -1)
                return base+6;
            else if (pos.z == latticeSites.z)
                return base+7;
            }
        }
    
    throw std::runtime_error("invalid site requested");
    
    }

void multirankQTensorLatticeModel::prepareSendingBuffer(int directionType)
    {

    if(!useGPU)
        {
        ArrayHandle<int> ht(types,access_location::host,access_mode::read);
        ArrayHandle<dVec> hp(positions,access_location::host,access_mode::read);
        ArrayHandle<int> iBuf(intTransferBufferSend,access_location::host,access_mode::readwrite);
        ArrayHandle<scalar> dBuf(doubleTransferBufferSend,access_location::host,access_mode::readwrite);
        int2 startStop = transferStartStopIndexes[directionType];
        int3 pos;
        int currentSite;
        for (int ii = startStop.x; ii <=startStop.y; ++ii)
            {
            getBufferInt3FromIndex(ii,pos,directionType,true);
            currentSite = positionToIndex(pos);
            iBuf.data[ii] = ht.data[currentSite];
            for(int dd = 0; dd < DIMENSION; ++dd)
                dBuf.data[DIMENSION*ii+dd] = hp.data[currentSite][dd];
            }
        }//end of CPU part
    else
        {
        }
    }

void multirankQTensorLatticeModel::readReceivingBuffer(int directionType)
    {
    int2 startStop = transferStartStopIndexes[directionType];
    int3 pos;
    int currentSite;
    if(!useGPU)
        {
        ArrayHandle<int> ht(types,access_location::host,access_mode::readwrite);
        ArrayHandle<dVec> hp(positions,access_location::host,access_mode::readwrite);
        ArrayHandle<int> iBuf(intTransferBufferReceive,access_location::host,access_mode::read);
        ArrayHandle<scalar> dBuf(doubleTransferBufferReceive,access_location::host,access_mode::read);
        for (int ii = startStop.x; ii <=startStop.y; ++ii)
            {
            getBufferInt3FromIndex(ii,pos,directionType,false);
            currentSite = positionToIndex(pos);
            ht.data[currentSite] = iBuf.data[ii];
            iBuf.data[ii] = ht.data[currentSite];
            for(int dd = 0; dd < DIMENSION; ++dd)
                hp.data[currentSite][dd] = dBuf.data[DIMENSION*ii+dd];
            }
        }//end of CPU part
    else
        {
        }
    }

/*!
During halo site communication, each rank will first completely fill the send buffer, and then send/receives within the buffers will be performed.
Finally, the entire receive buffer will be transfered into the expanded data arrays.
To facilitate this, a specific layout of the transfer buffers will be adopted for easy package/send/receive patterns:
the x = 0 face will be the first (Ly*Lz) elemetents, followed by the other faces, the edges, and finally the 8 corners.
 */
void multirankQTensorLatticeModel::determineBufferLayout()
    {
    int xFaces = latticeSites.z*latticeSites.y;
    int yFaces = latticeSites.z*latticeSites.x;
    int zFaces = latticeSites.x*latticeSites.y;
    //0: x = 0 face
    int2 startStop; startStop.x = 0; startStop.y = xFaces - 1;
    transferStartStopIndexes.push_back(startStop);
    //1: x = max face
    startStop.x = startStop.y+1; startStop.y += xFaces;
    transferStartStopIndexes.push_back(startStop);
    //2: y = 0
    startStop.x = startStop.y+1; startStop.y += yFaces;
    transferStartStopIndexes.push_back(startStop);
    //3: y = max face
    startStop.x = startStop.y+1; startStop.y += yFaces;
    transferStartStopIndexes.push_back(startStop);
    //4: z = 0
    startStop.x = startStop.y+1; startStop.y += zFaces;
    transferStartStopIndexes.push_back(startStop);
    //5: z = max face
    startStop.x = startStop.y+1; startStop.y += zFaces;
    transferStartStopIndexes.push_back(startStop);

    //6: x = 0, y = 0  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.z;
    transferStartStopIndexes.push_back(startStop);
    //7: x = 0, y = max  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.z;
    transferStartStopIndexes.push_back(startStop);
    //8: x = 0, z = 0  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.y;
    transferStartStopIndexes.push_back(startStop);
    //9: x = 0, z = max  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.y;
    transferStartStopIndexes.push_back(startStop);
    //10: x = max, y = 0  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.z;
    transferStartStopIndexes.push_back(startStop);
    //11: x = max, y = max  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.z;
    transferStartStopIndexes.push_back(startStop);
    //12: x = max, z = 0  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.y;
    transferStartStopIndexes.push_back(startStop);
    //13: x = max, z = max  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.y;
    transferStartStopIndexes.push_back(startStop);
    //14: y = 0, z = 0  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.x;
    transferStartStopIndexes.push_back(startStop);
    //15: y = 0, z = max  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.x;
    transferStartStopIndexes.push_back(startStop);
    //16: y = max, z = 0  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.x;
    transferStartStopIndexes.push_back(startStop);
    //17: y = max, z = max  edge
    startStop.x = startStop.y+1; startStop.y += latticeSites.x;
    transferStartStopIndexes.push_back(startStop);

    //18: x = 0, y = 0, z=0 corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //19: x = 0, y = 0, z=max corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //20: x = 0, y = max, z=0 corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //21: x = 0, y = max, z=max corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //22: x = max, y = 0, z=0 corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //23: x = max, y = 0, z=max corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //24: x = max, y = max, z=0 corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);
    //25: x = max, y = max, z=max corner
    startStop.x = startStop.y+1; startStop.y += 1;
    transferStartStopIndexes.push_back(startStop);

    printf("number of entries: %i\n",transferStartStopIndexes.size());
    for (int ii = 0; ii < transferStartStopIndexes.size(); ++ii)
        printf("%i, %i\n", transferStartStopIndexes[ii].x,transferStartStopIndexes[ii].y);
    intTransferBufferSend.resize(startStop.y);
    intTransferBufferReceive.resize(startStop.y);
    doubleTransferBufferSend.resize(DIMENSION*startStop.y);
    doubleTransferBufferReceive.resize(DIMENSION*startStop.y);
    }

/*!
maps between positions in the expanded lattice (base + halo sites) and the linear index of the position that site should reside in data.
respects the fact that the first N sites should be the base lattice, and the halo sites should all follow
*/
int multirankQTensorLatticeModel::indexInExpandedDataArray(int3 pos)
    {
    if(pos.x <0 && !xHalo)
        pos.x = latticeSites.x-1;
    if(pos.x ==latticeSites.x && !xHalo)
        pos.x = 0;
    if(pos.y <0 && !yHalo)
        pos.y = latticeSites.y-1;
    if(pos.y ==latticeSites.y && !yHalo)
        pos.y = 0;
    if(pos.z <0 && !zHalo)
        pos.z = latticeSites.z-1;
    if(pos.z ==latticeSites.z && !zHalo)
        pos.z = 0;

    if(pos.x < latticeSites.x && pos.y < latticeSites.y && pos.z < latticeSites.z && pos.x >=0 && pos.y >= 0 && pos.z >= 0)
        return latticeIndex(pos);

    //next consider the x = -1 face (total Ly * total Lz)
    int base = N;
    if(pos.x == -1)
        return base + pos.y + expandedLatticeSites.y*pos.z;
    //next the x + latticeSites.x + 1 face (note the off-by one fenceposting)
    base +=expandedLatticeSites.y*expandedLatticeSites.z;
    if(pos.x == latticeSites.x)
        return base + pos.y + expandedLatticeSites.y*pos.z;
    base +=expandedLatticeSites.y*expandedLatticeSites.z;
    //next consider the y = -1 face...  0 <=x < latticeSites, by -1 <= z <= latticeSites.z+1
    if(pos.y == -1)
        return base + pos.x + latticeSites.x*pos.z;
    base +=latticeSites.x*expandedLatticeSites.z;
    if(pos.y == latticeSites.y)
        return base + pos.x + latticeSites.x*pos.z;
    base +=latticeSites.x*expandedLatticeSites.z;

    //finally, the z-faces, for which x and y can only be 0 <= letter < latticeSites
    if(pos.z == -1)
        return base + pos.x + latticeSites.x*pos.y;
    base +=latticeSites.x*latticeSites.y;
    if(pos.z == latticeSites.z)
        return base + pos.x + latticeSites.x*pos.y;

    char message[256];
    sprintf(message,"inadmissible... {%i,%i,%i} = %i",pos.x,pos.y,pos.z,base);
    throw std::runtime_error(message);
    return -1;
    };

int multirankQTensorLatticeModel::getNeighbors(int target, vector<int> &neighbors, int &neighs, int stencilType)
    {
    if(stencilType==0)
        {
        neighs = 6;
        if(neighbors.size()!=neighs) neighbors.resize(neighs);
        if(!sliceSites)
            {
            int3 pos = latticeIndex.inverseIndex(target);
            neighbors[0] = indexInExpandedDataArray(pos.x-1,pos.y,pos.z);
            neighbors[1] = indexInExpandedDataArray(pos.x+1,pos.y,pos.z);
            neighbors[2] = indexInExpandedDataArray(pos.x,pos.y-1,pos.z);
            neighbors[3] = indexInExpandedDataArray(pos.x,pos.y+1,pos.z);
            neighbors[4] = indexInExpandedDataArray(pos.x,pos.y,pos.z-1);
            neighbors[5] = indexInExpandedDataArray(pos.x,pos.y,pos.z+1);

            }
        return target;
        };
    if(stencilType==1) //very wrong at the moment
        {
        UNWRITTENCODE("broken");
        int3 position = expandedLatticeIndex.inverseIndex(target);
        neighs = 18;
        if(neighbors.size()!=neighs) neighbors.resize(neighs);
        neighbors[0] = expandedLatticeIndex(wrap(position.x-1,expandedLatticeSites.x),position.y,position.z);
        neighbors[1] = expandedLatticeIndex(wrap(position.x+1,expandedLatticeSites.x),position.y,position.z);
        neighbors[2] = expandedLatticeIndex(position.x,wrap(position.y-1,expandedLatticeSites.y),position.z);
        neighbors[3] = expandedLatticeIndex(position.x,wrap(position.y+1,expandedLatticeSites.y),position.z);
        neighbors[4] = expandedLatticeIndex(position.x,position.y,wrap(position.z-1,expandedLatticeSites.z));
        neighbors[5] = expandedLatticeIndex(position.x,position.y,wrap(position.z+1,expandedLatticeSites.z));

        neighbors[6] = expandedLatticeIndex(wrap(position.x-1,expandedLatticeSites.x),wrap(position.y-1,expandedLatticeSites.y),position.z);
        neighbors[7] = expandedLatticeIndex(wrap(position.x-1,expandedLatticeSites.x),wrap(position.y+1,expandedLatticeSites.y),position.z);
        neighbors[8] = expandedLatticeIndex(wrap(position.x-1,expandedLatticeSites.x),position.y,wrap(position.z-1,expandedLatticeSites.z));
        neighbors[9] = expandedLatticeIndex(wrap(position.x-1,expandedLatticeSites.x),position.y,wrap(position.z+1,expandedLatticeSites.z));
        neighbors[10] = expandedLatticeIndex(wrap(position.x+1,expandedLatticeSites.x),wrap(position.y-1,expandedLatticeSites.y),position.z);
        neighbors[11] = expandedLatticeIndex(wrap(position.x+1,expandedLatticeSites.x),wrap(position.y+1,expandedLatticeSites.y),position.z);
        neighbors[12] = expandedLatticeIndex(wrap(position.x+1,expandedLatticeSites.x),position.y,wrap(position.z-1,expandedLatticeSites.z));
        neighbors[13] = expandedLatticeIndex(wrap(position.x+1,expandedLatticeSites.x),position.y,wrap(position.z+1,expandedLatticeSites.z));

        neighbors[14] = expandedLatticeIndex(position.x,wrap(position.y-1,expandedLatticeSites.y),wrap(position.z-1,expandedLatticeSites.z));
        neighbors[15] = expandedLatticeIndex(position.x,wrap(position.y-1,expandedLatticeSites.y),wrap(position.z+1,expandedLatticeSites.z));
        neighbors[16] = expandedLatticeIndex(position.x,wrap(position.y+1,expandedLatticeSites.y),wrap(position.z-1,expandedLatticeSites.z));
        neighbors[17] = expandedLatticeIndex(position.x,wrap(position.y+1,expandedLatticeSites.y),wrap(position.z+1,expandedLatticeSites.z));
        return target;
        }

    return target; //nope
    };

void multirankQTensorLatticeModel::parseDirectionType(int directionType, int &xyz, int &size1start, int &size1end, int &size2start, int &size2end,int &plane, bool sending)
    {
    //faces are easy
    if (directionType <= 5)
        {
        xyz = 0;//the plane has fixed x
        size1start=0; size1end = latticeIndex.sizes.y;
        size2start=0; size2end = latticeIndex.sizes.z;
        if(directionType == 2 || directionType ==3)
            {
            xyz = 1;//the plane has fixed y
            size1end = latticeIndex.sizes.x;
            }
        if(directionType == 4 || directionType ==5)
            {
            xyz = 2;//the plane has fixed y
            size1end = latticeIndex.sizes.x;
            size2end = latticeIndex.sizes.y;
            }
        if(sending && directionType <=5)
            {
            switch(directionType)
                {
                case 0: plane = 0; break;//smallest x plane
                case 1: plane = latticeSites.x-1; break;//largest x plane
                case 2: plane = 0; break;//smallest y plane
                case 3: plane = latticeSites.y-1; break;//largest y plane
                case 4: plane = 0; break;//smallest z plane
                case 5: plane = latticeSites.z-1; break;//largest z plane
                default:
                    throw std::runtime_error("negative directionTypes are not valid");
                }
            }
        else//receiving
            {
            switch(directionType)
                {
                case 0: plane = -1; break;//smallest x plane
                case 1: plane = latticeSites.x; break;//largest x plane
                case 2: plane = -1; break;//smallest y plane
                case 3: plane = latticeSites.y; break;//largest y plane
                case 4: plane = -1; break;//smallest z plane
                case 5: plane = latticeSites.z; break;//largest z plane
                default:
                    throw std::runtime_error("negative directionTypes are not valid");
                }
            };
        };
    //edges
    //6: x = -1, y = -1  edge
    if(directionType == 6)
        {
        xyz = 2 ;
        size2start = 0;
        size2start=latticeSites.z-1;
        if(sending)
            {
            plane = 0;
            size1start = 0; size1end = 1;
            }
        else
            {
            plane = -1;
            size1start = -1; size1end = 0;
            }
        }
    //7: x = -1, y = max  edge
    if(directionType == 7)
        {
        xyz = 2 ;
        size2start = 0;
        size2start=latticeSites.z-1;
        if(sending)
            {
            plane = latticeSites.y-1;
            size1start = 0; size1end = 1;
            }
        else
            {
            plane = latticeSites.y;
            size1start = -1; size1end = 0;
            }
        }
    //8: x = -1, z = -1  edge
    if(directionType ==8)
        {
        xyz = 1;
        size2start = 0;
        size2end = latticeSites.y-1;
        if(sending)
            {
            plane = 0;
            size1start = 0; size1end=1;
            }
        else
            {
            plane = -1;
            size1start = -1; size1end=0;
            }
        }
    //9: x = -1, z = max  edge
    if(directionType ==9)
        {
        xyz = 1;
        size2start = 0;
        size2end = latticeSites.y-1;
        if(sending)
            {
            plane = 0;
            size1start = latticeSites.z-1;latticeSites.z;
            }
        else
            {
            plane = -1;
            size1start = latticeSites.z;latticeSites.z+1;
            }
        }
    //10: x = max, y = -1  edge
    if(directionType == 10)
        {
        xyz = 2; 
        size2start = 0;
        size2start=latticeSites.z-1;
        if(sending)
            {
            plane = 0;
            size1start = latticeSites.x-1; size1end = latticeSites.x;
            }
        else
            {
            plane = -1;
            size1start = latticeSites.x; size1end = latticeSites.x+1;
            }
        }
    //11: x = max, y = max  edge
    if(directionType == 11)
        {
        xyz = 2 ;
        size2start = 0;
        size2start=latticeSites.z-1;
        if(sending)
            {
            plane = latticeSites.y-1;
            size1start = latticeSites.x-1; size1end = latticeSites.x;
            }
        else
            {
            plane = latticeSites.y;
            size1start = latticeSites.x; size1end = latticeSites.x+1;
            }
        }
    //12: x = max, z = -1  edge
    if(directionType ==12)
        {
        xyz = 1;
        size2start = 0;
        size2end = latticeSites.y-1;
        if(sending)
            {
            plane = latticeSites.x-1;
            size1start = 0; size1end = size1start +1;
            }
        else
            {
            plane = latticeSites.x;
            size1start = -1; size1end = size1start +1;
            }
        }
    //13: x = max, z = max  edge
    if(directionType ==13)
        {
        xyz = 1;
        size2start = 0;
        size2end = latticeSites.y-1;
        if(sending)
            {
            plane = latticeSites.x-1;
            size1start = latticeSites.z-1; size1end = size1start +1;
            }
        else
            {
            plane = latticeSites.x;
            size1start = latticeSites.z; size1end = size1start +1;
            }
        }
    //14: y = -1, z = -1  edge
    if(directionType ==14)
        {
        xyz = 0;
        size2start = 0;
        size2end = latticeSites.x-1;
        if(sending)
            {
            plane = 0;
            size1start = 0; size1end = size1start +1;
            }
        else
            {
            plane = -1;
            size1start = -1; size1end = size1start +1;
            }
        }
    //15: y = -1, z = max  edge
    if(directionType ==15)
        {
        xyz = 0;
        size2start = 0;
        size2end = latticeSites.x-1;
        if(sending)
            {
            plane = latticeSites.z-1;
            size1start = 0; size1end = size1start+1;
            }
        else
            {
            plane = latticeSites.z;
            size1start = -1; size1end = size1start+1;
            }
        }
    //16: y = max, z = -1  edge
    if(directionType ==15)
        {
        xyz = 0;
        size2start = 0;
        size2end = latticeSites.x-1;
        if(sending)
            {
            plane = 0;
            size1start = latticeSites.y-1; size1end = size1start+1;
            }
        else
            {
            plane = -1;
            size1start = latticeSites.y; size1end = size1start+1;
            }
        }
    //17: y = max, z = max  edge
    if(directionType ==17)
        {
        xyz = 0;
        size2start = 0;
        size2end = latticeSites.x-1;
        if(sending)
            {
            plane = latticeSites.z-1;
            size1start = latticeSites.y-1; size1end = size1start+1;
            }
        else
            {
            plane = latticeSites.z;
            size1start = latticeSites.y; size1end = size1start+1;
            }
        }
    if(directionType >17)
        {
        size1start = 0; size1end = 1;
        size2start = 0; size2end = 1;
        plane = 0;
        }
    }

void multirankQTensorLatticeModel::prepareSendData(int directionType)
    {
    if(directionType <=5 )//send entire faces
        {
        int xyz,size1start,size1end,size2start,size2end,plane;
        parseDirectionType(directionType,xyz,size1start,size1end,size2start,size2end,plane,true);
        int nTot = (size1end-size1start)*(size2end-size2start);
        transferElementNumber = nTot;
        if(intTransferBufferSend.getNumElements() < nTot)
            {
            intTransferBufferSend.resize(nTot);
            doubleTransferBufferSend.resize(DIMENSION*nTot);
            intTransferBufferReceive.resize(nTot);
            doubleTransferBufferReceive.resize(DIMENSION*nTot);
            }
        int currentSite;
        if(!useGPU)
            {
            ArrayHandle<int> ht(types,access_location::host,access_mode::read);
            ArrayHandle<dVec> hp(positions,access_location::host,access_mode::read);
            ArrayHandle<int> iBuf(intTransferBufferSend,access_location::host,access_mode::readwrite);
            ArrayHandle<scalar> dBuf(doubleTransferBufferSend,access_location::host,access_mode::readwrite);
            int idx = 0;
            int3 lPos;
            for (int ii = size1start; ii < size1end; ++ii)
                for (int jj = size2start; jj < size2end; ++jj)
                    {
                    if(xyz ==0)
                        {lPos.x = plane; lPos.y = ii; lPos.z = jj;}
                    if(xyz ==1)
                        {lPos.x = ii; lPos.y = plane; lPos.z = jj;}
                    if(xyz ==2)
                        {lPos.x = ii; lPos.y = jj; lPos.z = plane;}
                    currentSite = indexInExpandedDataArray(lPos);
                    iBuf.data[idx] = ht.data[currentSite];
                    for(int dd = 0; dd < DIMENSION; ++dd)
                        dBuf.data[DIMENSION*idx+dd] = hp.data[currentSite][dd];
                    idx+=1;
                    }
            }
        else
            {//GPU copy routine
            bool sending = true;
            ArrayHandle<int> dt(types,access_location::device,access_mode::readwrite);
            ArrayHandle<dVec> dp(positions,access_location::device,access_mode::readwrite);
            ArrayHandle<int> iBuf(intTransferBufferReceive,access_location::device,access_mode::read);
            ArrayHandle<scalar> dBuf(doubleTransferBufferReceive,access_location::device,access_mode::read);
            gpu_mrqtlm_buffer_data_exchange(sending,dt.data,dp.data,iBuf.data,dBuf.data,
                    size1start,size1end,size2start,size2end,
                    xyz,plane,latticeSites,expandedLatticeSites,latticeIndex,
                    xHalo,yHalo,zHalo);
            }
        }//end construction for sending faces
    }

void multirankQTensorLatticeModel::receiveData(int directionType)
    {
    if(directionType <=5 )//send entire faces
        {
        int xyz,size1start,size1end,size2start,size2end,plane;
        parseDirectionType(directionType,xyz,size1start,size1end,size2start,size2end,plane,false);
        int currentSite;
        if(!useGPU)
            {
            ArrayHandle<int> ht(types,access_location::host,access_mode::readwrite);
            ArrayHandle<dVec> hp(positions,access_location::host,access_mode::readwrite);
            ArrayHandle<int> iBuf(intTransferBufferReceive,access_location::host,access_mode::read);
            ArrayHandle<scalar> dBuf(doubleTransferBufferReceive,access_location::host,access_mode::read);
            int idx = 0;
            int3 lPos;
            for (int ii = size1start; ii < size1end; ++ii)
                for (int jj = size2start; jj < size2end; ++jj)
                    {
                    if(xyz ==0)
                        {lPos.x = plane; lPos.y = ii; lPos.z = jj;}
                    if(xyz ==1)
                        {lPos.x = ii; lPos.y = plane; lPos.z = jj;}
                    if(xyz ==2)
                        {lPos.x = ii; lPos.y = jj; lPos.z = plane;}
                    currentSite = indexInExpandedDataArray(lPos);
                    ht.data[currentSite] = iBuf.data[idx];
                    for(int dd = 0; dd < DIMENSION; ++dd)
                        hp.data[currentSite][dd] = dBuf.data[DIMENSION*idx+dd];
                    idx+=1;
                    }
            }
        else
            {//GPU copy routine
            bool sending = false;
            ArrayHandle<int> dt(types,access_location::device,access_mode::readwrite);
            ArrayHandle<dVec> dp(positions,access_location::device,access_mode::readwrite);
            ArrayHandle<int> iBuf(intTransferBufferReceive,access_location::device,access_mode::read);
            ArrayHandle<scalar> dBuf(doubleTransferBufferReceive,access_location::device,access_mode::read);
            gpu_mrqtlm_buffer_data_exchange(sending,dt.data,dp.data,iBuf.data,dBuf.data,
                    size1start,size1end,size2start,size2end,
                    xyz,plane,latticeSites,expandedLatticeSites,latticeIndex,
                    xHalo,yHalo,zHalo);
            }
        }//end construction for sending faces
    }
