import os

dirLoc= "E:\\nRF52832\\SDK\\components"

os.walk(dirLoc)

#for root, dirs, files in os.walk(dirLoc):
for dirs in os.walk(dirLoc):
   # print(root)
    print(dirs[0])
  #  print(files)

print("line space--------------")
import os
output = [dI for dI in os.listdir(dirLoc) if os.path.isdir(os.path.join(dirLoc,dI))]
print(output)


for i,j,y in os.walk('.'):
    print(i)