#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
//Multiply two 4*4 matrices.
const int stepXSize = 4;
const int stepYSize = 4;
float16 multMat (float16 matA, float16 matB) {
    float16 matC;        
    for (int x = 0; x < 4; ++x) {
	for (int y = 0; y < 4; ++y) {
            float value = 0;
	    for (int k = 0; k < 4; ++k) {
		float elementA = matA[y * 4 + k];
                float elementB = matB[k * 4 + x];
		value += elementA * elementB;
	    }
	    matC[y * 4 + x] = value;		
        }
    }
    return matC;
}
//Multiply a 4*4 matrix with a vec4.
float4 multVec (float16 matA, float4 vecB) {
   float4 vecC;
   for (int i = 0; i < 4; ++i) {
        float value = 0;
	for (int j = 0; j < 4; ++j) {
             value += vecB[j] * matA[i][j];
	}
	vecC[i] = value;
   }
   return vecC;
}
//Transpose a 4*4 matrix.
float16 transpose(float16 matA) {
	float16 matT
	for (int i = 0; i < 4; ++i) {        	
		for (int j = 0; j < 4; ++j) {
			matT[i][j] = matA[j][i];
		}
	}
	return matT;
}
float  det (float4 mat) {
	return mat[0] * mat[2] - mat[1] * mat[3];
}
float min (float3 vec) {
	float cmin = vec.x;
	for (int i = 1; i < 3; ++i) {
		if (vec[i] < cmin)
			cmin = vec[i];
	}
	return cmin;
}
float max (float3 vec) {
	float cmax = vec.x;
	for (int i = 1; i < 3; ++i) {
		if (vec[i] > cmax)
			cmax = vec[i];
	}
	return cmax;
}
float3 initEdge (const float2& v0, const float2& v1, const float2& origin, float2& oneStep) {
       // Edge setup
       int a = v0.y - v1.y, b = v1.x - v0.x;
       int c = v0.x*v1.y - v0.y*v1.x;

       // Step deltas
       oneStep = (float2) (math::Vec3f(a * stepXSize, a * stepXSize, a * stepXSize, a * stepXSize,
			   math::Vec3f(b * stepYSize, b * stepYSize, b * stepYSize, b * stepYSize);

       // x/y values for initial pixel block
       float3 x = float3 (origin.x, origin.x, origin.x, origin.x) + float3(0,1,2,3);
       float3 y = float3 (origin.y, origin.y, origin.y, origin.y) + float3(0,1,2,3);

       // Edge function values at origin
       return float3(a, a, a, a)*x + float3(b, b, b, b)*y + float3(c, c, c, c);
}
//Convert vertex position from object space toviewport space.
__kernel void vertexShader(__global float* vPosX, __global float* vPosY, __global float* vPosZ, __global float* vPosW,
                           __global unsigned int* indices,  __global unsigned int numIndices, __global unsigned int* baseIndices,
			   __global unsigned int* baseVertices,  __global float* transfMatrices, __global float16 projMatrix,
                           __global float16 viewMatrix,  __global float16 viewportMatrix) {
        size_t instanceID = get_global_id(0);
	float16 transfMatrix;
	for (int i = 0; i < 16; i++) {
	    transfMatrix[i] = transfMatrices[instanceID*16+i];
	}
	for (unsigned int offset = 0; offset < numIndices[instanceID]; ++offset) {		
		float4 position = (float4) (vPosX[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset], 
					    vPosY[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset], 
					    vPosZ[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset], 
                                            vPosW[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset]);
		float4 worldcoords = multVec(transfMatrix, position);
		float4 viewcoords = multVec(viewMatrix, worldcoords);
        	float4 clipcoords = multVec(projMatrix, viewcoords);
		float4 ndcCoords = clipcoords / clipcoords.w;
		position = multVec(viewportMatrix, ndcCoords);
		vPosX[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset] = abs(position.x);
       	 	vPosY[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset] = abs(position.y);
        	vPosZ[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset] = abs(position.z);
        	vPosW[baseVertices[instanceID]+indices[baseIndices[instanceID]+offset] = position.w; 
	}	
}
__kernel void geometryShader (__global float* vPosX, __global float* vPosY, __global float* vPosZ, __global float* vPosW,
			       __global unsigned int* indices,  __global unsigned int numIndices, __global unsigned int* baseIndices,
			       __global unsigned int* baseVertices, __global unsigned int* nbVerticesPerFace, __global unsigned int* loop) {
	size_t tid = get_global_id(0);
	size_t tid2;
	if (get_global_id(0) == numIndices-1) {
	    if (loop[0] == 1) {
		tid2 = 0;
		float centerLineX=0, centerLineY=0, centerLineZ=0, centerLineW=0;
		float4 v1(vPosX[baseVertices[tid]+indices[baseIndices[tid]], vPosY[baseVertices[tid]+indices[baseIndices[tid]], 
		vPosZ[baseVertices[tid]+indices[baseIndices[tid]],vPosW[baseVertices[tid]+indices[baseIndices[tid]]);
		float4 v2(vPosX[baseVertices[tid]+indices[baseIndices[tid]], vPosY[baseVertices[tid]+indices[baseIndices[tid]], 
		vPosZ[baseVertices[tid]	+ indices[baseIndices[tid]+tid2],vPosW[baseVertices[tid]+indices[baseIndices[tid]]);
		centerLineX = (v1.x + v2.x) * 0.5;
		centerLineY = (v1.y + v2.y) * 0.5;
		centerLineZ = (v1.z + v2.z) * 0.5;
		centerLineW = (v1.w + v2.w) * 0.5;
		vPosX[tid*2+1] = centerLineX;
		vPosY[tid*2+1] = centerLineY;
		vPosZ[tid*2+1] = centerLineZ;
		vPosW[tid*2+1] = centerLineW;
	    } else {
		float4 v(vPosX[baseVertices[tid]+indices[baseIndices[tid]], vPosY[baseVertices[tid]+indices[baseIndices[tid]], 
		vPosZ[baseVertices[tid]+indices[baseIndices[tid]],vPosW[baseVertices[tid]+indices[baseIndices[tid]]);
		vPosX[tid*2] = v.x;
		vPosY[tid*2] = v.y;
		vPosZ[tid*2] = v.z;
		vPosW[tid*2] = v.w;		
	    }
	} else {
		tid2 = tid + 1;
		float centerLineX=0, centerLineY=0, centerLineZ=0, centerLineW=0;
		float4 v1(vPosX[baseVertices[tid]+indices[baseIndices[tid]], vPosY[baseVertices[tid]+indices[baseIndices[tid]], 
		vPosZ[baseVertices[tid]+indices[baseIndices[tid]],vPosW[baseVertices[tid]+indices[baseIndices[tid]]);
		float4 v2(vPosX[baseVertices[tid]+indices[baseIndices[tid]], vPosY[baseVertices[tid]+indices[baseIndices[tid]], 
		vPosZ[baseVertices[tid]	+ indices[baseIndices[tid]+tid2],vPosW[baseVertices[tid]+indices[baseIndices[tid]]);
		centerLineX = (v1.x + v2.x) * 0.5;
		centerLineY = (v1.y + v2.y) * 0.5;
		centerLineZ = (v1.z + v2.z) * 0.5;
		centerLineW = (v1.w + v2.w) * 0.5;
		vPosX[tid*2] = v1.x;
		vPosY[tid*2] = v1.y;
		vPosZ[tid*2] = v1.z;
		vPosW[tid*2] = v1.w;
		vPosX[tid*2+1] = centerLineX;
		vPosY[tid*2+1] = centerLineY;
		vPosZ[tid*2+1] = centerLineZ;
		vPosW[tid*2+1] = centerLineW;
	}
	nbVerticesPerFace[1] = nbVerticesPerFace[0] * 2;
}
__kernel void tesslationShader(__global float* vPosX, __global float* vPosY, __global float* vPosZ, __global float* vPosW,
			       __global unsigned int* indices,  __global unsigned int numIndices, __global unsigned int* baseIndices,
			       __global unsigned int* baseVertices, __global unsigned int* nbVerticesPerFaces, __global unsigned int* loop,
			       __global unsigned float* centersX, __global unsigned float* centersY,__global unsigned float* centersZ,
			       __global unsigned float* centersW) {
	size_t tid = get_global_id(0);
	float centerX=0, centerY=0, centerZ=0, centerW = 0;
	for (unsigned int i = 0; i < nbVerticesPerFace[1]; i++) {
	     if (i * 0.5 + nbVerticesPerFace[0] < numIndices) {
	     	centerX += vPosX[baseVertices[tid]+indices[baseIndices[tid]+i];
	     	centerY += vPosY[baseVertices[tid]+indices[baseIndices[tid]+i];
	     	centerZ += vPosZ[baseVertices[tid]+indices[baseIndices[tid]+i];
             	centerW += vPosW[baseVertices[tid]+indices[baseIndices[tid]+i];	
	     }
	}
	centerX = centerX / nbVerticesPerFace[1];
	centerY = centerY / nbVerticesPerFace[1];
	centerZ = centerZ / nbVerticesPerFace[1];
	centerW = centerW / nbVerticesPerFace[1];
	centersX[tid] = centerX;
	centersY[tid] = centerY;
	centersZ[tid] = centerZ;
	centersW[tid] = centerW;
}
__kernel void fragmentShader(__global float* vPosX, __global float* vPosY, __global float* vPosZ, __global float* vPosW,
			     __global unsigned int* vColRed, __global unsigned int* vColBlue, __global unsigned int* vColGreen, 			     __global unsigned int* vColAlpha, __global int* vTCU, __global int* vTCV, 
			     __global unsigned int* indices,  __global unsigned int numIndices, __global unsigned int* baseIndices,
			     __global unsigned int* baseVertices, __global float* redBuffer, __global float* blueBuffer, 
			     __global float* greenBuffer, __global float* alphaBuffer, __global float* depthBuffer,
			     __global unsigned float* centersX, __global unsigned float* centersY, __global float* centersZ,
			     __global unsigned float* centersW, __global float16 viewportMatrix, __global float16 texMatrix) {
	size_t tid = get_global_id(0);	
	float4 p1, p2, p3;
	p1 = (float4) (centersX[tid], centersY[tid], centersZ[tid], centersW[tid]);
	for (unsigned int i = 0; i < nbVerticesPerFace[1]; i++) {
		int i2 = (i + 1 >= nbVerticesPerFace[1]) ? 0 : i+1;
	        p2 = (float4) (vPosX[baseVertices[tid]+indices[baseIndices[tid]+i,vPosY[baseVertices[tid]+indices[baseIndices[tid]+i,
			       vPosZ[baseVertices[tid]+indices[baseIndices[tid]+i,vPosW[baseVertices[tid]+indices[baseIndices[tid]+i);
		p3 = (float4) (vPosX[baseVertices[tid]+indices[baseIndices[tid]+i2,vPosY[baseVertices[tid]+indices[baseIndices[tid]+i2,
			       vPosZ[baseVertices[tid]+indices[baseIndices[tid]+i2,vPosW[baseVertices[tid]+indices[baseIndices[tid]+i2);
		
	}
	float3 xs (p1.x, p2.x, p3.x);
	float3 ys (p1.y, p2.y, p3.y);
	float minX = min(xs);
	float minY = min(ys);
        float maxX = max(xs);
	float maxY = max(ys);
	int a01 = p1.y - p2.y, b01 = p2.x - p1.x;
        int a12 = p2.y - p3.y, b12 = p3.x - p2.x;
        int a20 = p3.y - p1.y, b20 = p1.x - p3.x;
        float2 p(minX, minY);
	float2 steps_e12, steps_e20, steps_e01;
	float3 w0_row = initEdge(p2, p3, p, steps_e12);
        float3 w1_row = initEdge(p3, p1, p, steps_e20);
        float3 w2_row = initEdge(p1, p2, p, steps_e01);
	int viewportWidth = viewportMatrix[0] * 2;
	int viewportHeight = viewportMatrix[5] * 2;
        int viewportDepth = viewportMatrix[10] * 2;
	int viewportX = viewportMatrix[3] - viewportMatrix[0];
	int viewportY = viewportMatrix[7] - viewportMatrix[5];
	int viewportZ = viewportMatrix[11] - viewportMatrix[10];
	for (p.y = minY; p.y < maxY; p += stepYSize) {
		float3 w0 = w0_row;
		float3 w1 = w1_row;
		float3 w2 = w2_row;
		for (p.x = minX; p.x < maxX; p.x += stepXSize) {
		    int3 mask ((int) w0.x | (int) w1.x | (int) w2.x,
			       (int) w0.y | (int) w1.y | (int) w2.y,
			       (int) w0.z | (int) w1.z | (int) w2.z,
			       (int) w0.w | (int) w1.w | (int) w2.w);
		    if (mask.x < 0 || mask.y < 0 || mask.z < 0 || mask.w < 0) {
		        for (unsigned int y = p.y; y < p.y + Edge::stepYSize; y++) {
		            for (unsigned int x = p.x; x < p.x + Edge::stepXSize; x++) {
				float4 m  (p1.x - p3.x, p2.x - p3.x,
		                          p1.y - p3.y, p2.y - p3.y);
		                float u = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) / det(m);
		                float v = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) / det(m);
		                float w = 1 - u - v;
			    }
		        }
		    }
		    w0 += steps_e12.x;
                    w1 += steps_e20.x;
                    w2 += steps_e01.x;	
		}
                w0_row += steps_e12.y;
		w1_row += steps_e20.y;
		w2_row += steps_e01.y;
        }	
}
		


