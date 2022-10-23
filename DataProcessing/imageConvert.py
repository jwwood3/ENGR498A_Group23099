import os
import skimage.io as io
import skimage

IMG_RES = 100

def convertImage(filename):
    #filename = os.path.join(skimage.data_dir, filename)
    img = io.imread(filename)
    grayImg = skimage.color.rgb2gray(skimage.color.rgba2rgb(img))
    grayImg = skimage.transform.rescale(grayImg, (IMG_RES/len(grayImg),IMG_RES/len(grayImg[0])), anti_aliasing=True)
    x = []
    y = []
    z = []
    for i in range(len(grayImg)):
        for j in range(len(grayImg[i])):
            x.append(i)
            y.append(j)
            z.append(grayImg[i][j])
    return x,y,z

def outputImage(x,y,z,filename):
    f = open(filename, "w")
    for i in range(len(x)):
        f.write(str(x[i]) + " " + str(y[i]) + " " + str(z[i]) + "\n")
    f.close()

if __name__ == "__main__":
    for i in range(6):
        x,y,z=convertImage("testImage"+str(i+1)+".png")
        outputImage(x,y,z,"testImage"+str(i+1)+".xyz")