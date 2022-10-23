import os
import graphProcess
import sendDataMulti
def processAll(dir):
    for i,filename in enumerate(os.listdir(dir)):
        file = os.path.join(dir,filename)
        if os.path.isfile(file):
            x,y,z = [],[],[]
            if file.endswith(".xyz") and not file.endswith("_min.xyz"):
                x,y,z=graphProcess.findMin(file,image=False,display=False,save=False,gray=True,summary=False)
            elif file.endswith(".png") or file.endswith(".jpg"):
                x,y,z=graphProcess.findMin(file,image=True,display=False,save=False,gray=True,summary=False)
            else:
                continue
            #graphProcess.outputImage(x,y,z,file.split(".")[0] + "_min.xyz")
            sendDataMulti.send(z)
            break

if __name__=="__main__":
	processAll(".")
