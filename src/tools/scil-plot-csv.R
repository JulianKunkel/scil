#!/usr/bin/env Rscript

# This file is part of SCIL.
#
# SCIL is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SCIL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with SCIL.  If not, see <http://www.gnu.org/licenses/>.


plotme3D = function (file, d){
  library(plot3D) # install.packages("plot3D")
  pdf(paste(file, ".pdf", sep=""))
  image2D(d, rasterImage = TRUE, contour = TRUE, shade = 0.5, lphi = 0)
  dev.off()

  png(paste(file, "-3D1.png", sep=""), width=1024, height=1024, units="px", pointsize = 24)
  persp3D(z=d, contour = list(side = c("zmax", "z")), theta = 0, phi=35)
  dev.off()

  png(paste(file, "-3D2.png", sep=""), width=1024, height=1024, units="px", pointsize = 24)
  persp3D(z=d, contour = list(side = c("zmax", "z")), theta = 90, phi=45)
  dev.off()

  png(paste(file, "-3D3.png", sep=""), width=1024, height=1024, units="px", pointsize = 24)
  persp3D(z=d, contour = list(side = c("zmax", "z")), theta = -90, phi=45)
  dev.off()
}

options <- commandArgs(trailingOnly = TRUE)

if( length(options) < 1){
  print("Synopsis: <file.csv>")
  quit(save="no", status=1)
}

file = options[1]

d = read.csv(file, header=F)
head = d[1,]
dims = 1

dimx = head[1][,]

if (! is.na(head[2])){
  dims = 2
  dimy = head[2][,]
}
if (! is.na(head[3])){
  dims = 3
  dimz = head[3][,]  
}

d = as.matrix(d[2:nrow(d),])

print(sprintf("min: %f max: %f", min(d), max(d)))
print("Be aware: this visulation tool takes a long time if data is random")

if(dims == 1){
  pdf(paste(file, ".pdf", sep=""))
  plot(d[1,], ylab="Value")
  dev.off()
}
if(dims == 2){
  # image(d)
  plotme3D(file, d)
}

if(dims == 3){
  print("We plot a 2D slice")
  p = d[1:dimy+dimy,]
  plotme3D( paste(file, "1", sep=""), p)

  p = matrix(d[1:nrow(d),2] , ncol=dimz)
  plotme3D( paste(file, "2", sep=""), p)
}
