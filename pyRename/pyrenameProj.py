


#searchfile = open("blinky_pca10040_s132.txt", "r")
#for line in searchfile:
#    if "../../" in line: print(line)
#searchfile.close()

import fileinput
from collections import OrderedDict
import os

def replace_all(text, dic):
    for i, j in dic.items():
        text = text.replace(i, j)
    return text

projName = "GPS_uart"
projNewName = "GPS_EINK"

dic = OrderedDict()
dic["macros=\"CMSIS_CONFIG_TOOL=../../../../../../external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar"] =  "macros=\"CMSIS_CONFIG_TOOL=E:/nRF52832/SDK/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar"
dic["../../../../../../"]= "$(SDK)/"
dic["../../../config"   ]= "$(SDK)/config"
dic["../config"         ]= "Code/config"
dic["../../../main"     ]= "Code/main"

dic[projName] = projNewName

print(dic)

filename = "E:\\nRF52832\\Projects\\pyRename\\" + projName + ".emProject"
with fileinput.FileInput(filename, inplace=True) as file:
    for line in file:
        print(replace_all(line,dic).rstrip("\n\r"))


try:
    os.remove(projNewName+ ".emProject")
except Exception:
    pass

try:
    os.remove(projNewName+ ".emSession")
except Exception:
    pass

try:
    os.rename(projName+ ".emProject", projNewName+ ".emProject")
except Exception:
    pass

try:
    os.rename(projName+ ".emSession", projNewName+ ".emSession")
except Exception:
    pass

try:
    os.remove(projName + ".emProject")
except Exception:
    pass
try:
    os.remove(projName + ".emSession")
except Exception:
    pass

        #print(line.replace(projName, projNewName), end='')


