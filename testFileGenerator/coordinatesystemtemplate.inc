/*
* Copyright (c) 2011 Nathanael Hübbe
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

#declare flipYAxis = true;
#declare makeTangentPlane = true;

#macro makeStage()
	camera {
		location <1.3, 1.8, -2.6>
		up y
		right x*image_width/image_height
		look_at <-0.15, -0.3, -.05>
	}
	
	//camera light
	light_source {
		<1.3, 1.8, -2.5>
		color rgb .4
	}
	
	//main light
	light_source {
		<-1200, 1500, -1000>
		color rgb 1
	}
	
	//sea level
	plane {
		y, 0
		texture { pigment { color rgbt <0, 0, 1, 0.6> } }
		clipped_by { box { -1 1 } }
	}
	
	//the three axis arrows
	#declare arrow = union {
		cylinder { -x, x, 0.01 }
		cone { x, 0.05, x*1.1, 0 }
		texture { pigment { color rgb x+y } }
	}
	
	object { arrow }
	object { arrow rotate 90*z }
	#if (flipYAxis)
		object { arrow rotate 90*y }
	#else
		object { arrow rotate -90*y }
	#end
#end

#macro boundingLine(x0, x1, intervalCount)
	union {
		cylinder { x0, x1, 0.005 }
		sphere { x0, 0.005 }
		sphere { x1, 0.005 }
		texture { pigment { color rgb 1 } }
	}
	#local d = (x1-x0)/intervalCount;
	#local i = 1;
	#while (i < intervalCount)
		#local curPoint = x0+d*i;
		cylinder {
			curPoint-0.05*d, curPoint+0.05*d, 0.0051
			texture { pigment { color rgb x } }
		}
		#local i = i+1;
	#end
#end

#macro bounding(coord, maxValue, intervalCount)
	#local inverted = false;
	#if (coord.x)
		#if (coord.x < 0)
			#local inverted = true;
		#end
		#local a = y;
		#local b = z;
	#else #if (coord.y)
		#if (coord.y < 0)
			#local inverted = true;
		#end
		#local a = x;
		#local b = z;
	#else
		#if (coord.z < 0)
			#local inverted = true;
		#end
		#local a = x;
		#local b = y;
	#end #end
	boundingLine(-coord-a-b, coord-a-b, intervalCount)
	boundingLine(-coord  -b, coord  -b, intervalCount)
	boundingLine(-coord+a-b, coord+a-b, intervalCount)
	boundingLine(-coord+a  , coord+a  , intervalCount)
	boundingLine(-coord+a+b, coord+a+b, intervalCount)
	boundingLine(-coord  +b, coord  +b, intervalCount)
	boundingLine(-coord-a+b, coord-a+b, intervalCount)
	boundingLine(-coord-a  , coord-a  , intervalCount)
	#local caption = text {
		ttf "crystal.ttf", str(maxValue, 1, 0), 0.01, 0
		texture { pigment { color rgb <1,1,0> } }
		scale 0.15
	}
	#local x0 = min_extent(caption);
	#local x1 = max_extent(caption);
	#local captionSize = x1-x0;
	#if (inverted)
		#local dx = 1.1*coord + captionSize*coord - 0.5*captionSize*(a+b);
	#else
		#local dx = 1.15*coord - 0.5*captionSize*(a+b);
	#end
	object { caption translate dx }

	#local caption = text {
		ttf "crystal.ttf", str(-maxValue, 1, 0), 0.01, 0
		texture { pigment { color rgb <1,1,0> } }
		scale 0.15
	}
	#local x0 = min_extent(caption);
	#local x1 = max_extent(caption);
	#local captionSize = x1-x0;
	#if (inverted)
		#local dx = -1.15*coord - 0.5*captionSize*(a+b);
	#else
		#local dx = -1.1*coord - captionSize*coord - 0.5*captionSize*(a+b);
	#end
	object { caption translate dx }
#end

#macro makeBoundaries(ranges, intervalCounts)
	bounding(x, ranges.x, intervalCounts.x)
	#if (flipYAxis)
		bounding(-z, ranges.y, intervalCounts.y)
	#else
		bounding(z, ranges.y, intervalCounts.y)
	#end
	bounding(y, ranges.z, intervalCounts.z)
#end

#macro makeGraph(data, ranges, intervalCounts)
	#local uSize = dimension_size(data, 1);
	#local vSize = dimension_size(data, 2);
	//get the range of the data
	#local maxAbs = 0;
	#local i = 0;
	#while (i < uSize)
		#local j = 0;
		#while (j < vSize)
			#local temp = data[i][j];
			#if (temp > maxAbs)
				#local maxAbs = temp;
			#else #if (-temp > maxAbs)
				#local maxAbs = -temp;
			#end #end
			#local j = j+1;
		#end
		#local i = i+1;
	#end
	//determine the vertical range of the coordinate system
	#local zRange = 1;
	#while (zRange < maxAbs)
		#local zRange = 10*zRange;
	#end
	#local continue = (zRange >= maxAbs);
	#while (continue)
		#local zRange = zRange/10;
		#local continue = (zRange >= maxAbs);
		#if (!zRange)
			#local continue = false;
		#end
	#end
	#local factor = int(maxAbs/zRange + 1);
	#local zRange = zRange*factor;
	//make the stage & the bounds
	makeStage()
	makeBoundaries(<ranges.x, ranges.y, zRange>, <intervalCounts.x, intervalCounts.y, 2*factor>)
	//scale down the data
	#local i = 0;
	#while (i < uSize)
		#local j = 0;
		#while (j < vSize)
			#local data[i][j] = data[i][j]/zRange;
			#local j = j+1;
		#end
		#local i = i+1;
	#end
	//flip the data if the y-axis should be fliped
	#if (flipYAxis)
		#local i = 0;
		#while (i < uSize)
			#local j = 0;
			#while (2*j < vSize-1)
				#local temp = data[i][j];
				#local data[i][j] = data[i][vSize-1-j];
				#local data[i][vSize-1-j] = temp;
				#local j = j+1;
			#end
			#local i = i+1;
		#end
	#end
	
	//make the graph surface
	mesh2 {
		vertex_vectors {
			uSize*vSize
			#local uScale = 2/(uSize-1);
			#local vScale = 2/(vSize-1);
			#local i = 0;
			#while (i < uSize)
				#local j = 0;
				#while (j < vSize)
					, <i*uScale-1, data[i][j], j*vScale-1>
					#local j = j+1;
				#end
				#local i = i+1;
			#end
		}
		normal_vectors {
			uSize*vSize
			#local uScale = 2/(uSize-1);
			#local vScale = 2/(vSize-1);
			#local i = 0;
			#while (i < uSize)
				#local j = 0;
				#while (j < vSize)
					#if (i)
						#if (i < (uSize-1))
							#local tangent1 = <2*uScale, data[i+1][j]-data[i-1][j], 0>;
						#else
							#local tangent1 = <uScale, data[i][j]-data[i-1][j], 0>;
						#end
					#else
						#local tangent1 = <uScale, data[i+1][j]-data[i][j], 0>;
					#end
					#if (j)
						#if (j < (vSize-1))
							#local tangent2 = <0, data[i][j+1]-data[i][j-1], 2*vScale>;
						#else
							#local tangent2 = <0, data[i][j]-data[i][j-1], vScale>;
						#end
					#else
						#local tangent2 = <0, data[i][j+1]-data[i][j], vScale>;
					#end
					, vcross(tangent1, tangent2)
					#local j = j+1;
				#end
				#local i = i+1;
			#end
		}
		face_indices {
			2*(uSize-1)*(vSize-1)
			#local i = 0;
			#while (i < (uSize-1))
				#local j = 0;
				#while (j < (vSize-1))
					#local vertex1 = i*vSize + j;
					#local vertex2 = vertex1 + 1;
					#local vertex3 = vertex1 + vSize;
					#local vertex4 = vertex3 + 1;
					, <vertex1, vertex2, vertex3>, <vertex2, vertex3, vertex4>
					#local j = j+1;
				#end
				#local i = i+1;
			#end
		}
		texture { 
			checker
			texture { pigment { color rgb y } }
			texture { pigment { color rgb y*0.5 } }
			translate -0.5*y
			scale <0.1, 2, 0.1>
		}
	}
	
	//make the tangent plane (if desired)
	#if (makeTangentPlane)
		#local center = <uSize-1, vSize-1>/2;
		#local tangent1 = <2*uScale, data[center.u+1][center.v]-data[center.u-1][center.v], 0>;
		#local tangent2 = <0, data[center.u][center.v+1]-data[center.u][center.v-1], 2*vScale>;
		#local normalVector = vcross(tangent2, tangent1);
		#local normalVector = normalVector/sqrt(vdot(normalVector, normalVector));
		#local d = vdot(normalVector, <0, data[center.u][center.v], 0>);
		plane {
			normalVector, d
			clipped_by { box { -1, 1 } }
			texture { pigment { color rgbt <1, 0, 0, 0.6> } }
		}
	#end
#end

//test code
#if (0)
	#declare flipYAxis = false;

	#declare data = array[61][61];
	#local i = 0;
	#while (i < 61)
		#local a = (i-30)/10;
		#local j = 0;
		#while (j < 61)
			#local b = (j-30)/10;
//			#local value = a*exp(-(a*a+b*b)/2);
//			#if (0)	//(value <= 0)
//				#local value = 0.0001;
//			#end
			#local value = - a*a - b*b - 4*a + 2*b + 15;
			#declare data[i][j] = value;
			#local j = j+1;
		#end
		#local i = i+1;
	#end
	
	makeGraph(data, <25, 25>, <10, 10>)
#end

