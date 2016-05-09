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


options <- commandArgs(trailingOnly = TRUE)

if( length(options) < 1){
  print("Synopsis: <file.csv>")
  quit(save="no", status=1)
}

file = options[1]

d = read.csv(file, header=F)
head = d[1,]
dims = 1
if (! is.na(head[2])){
  dims = 2
}
if (! is.na(head[3])){
  dims = 3
  dimz = head[3][,]
  dimy = head[2][,]
}


d = as.matrix(d[2:nrow(d),])

print(sprintf("min: %f max: %f", min(d), max(d)))

pdf(paste(file, ".pdf", sep=""))

if(dims == 1){
  plot(d[1,], ylab="Value")
}
if(dims == 2){
  # image(d)
  library(plot3D) # install.packages("plot3D")
  image2D(d, rasterImage = TRUE, contour = TRUE, shade = 0.5, lphi = 0)
  dev.off()

  png(paste(file, "-3D.png", sep=""), width=1024, height=1024, units="px", pointsize = 24)
  persp3D(z=d, contour = list(side = c("zmax", "z")))
  dev.off()
}
if(dims == 3){
  print("We plot a 2D slice")
  library(plot3D) # install.packages("plot3D")
  p = d[1:dimy,]
  image2D(p, rasterImage = TRUE, contour = TRUE, shade = 0.5, lphi = 0)
  dev.off()

  png(paste(file, "-3D.png", sep=""), width=1024, height=1024, units="px", pointsize = 24)
  persp3D(z=p, contour = list(side = c("zmax", "z")))
  dev.off()
}
