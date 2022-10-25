import numpy as np
import scipy.interpolate as interp
import skimage.io as io
import skimage

IMG_RES = 100
XSCALE = 3
YSCALE = 3
ZSCALE = 3

def findMin(filename,image=False):
    x,y,z = [],[],[]
    if image:
        x,y,z = convertImage(filename)
    else:
        x,y,z = np.genfromtxt(filename, delimiter=" ", unpack=True, autostrip=True)
    x2,y2,z2 = graphProcess(x,y,z, XSCALE, YSCALE, ZSCALE)
    x4,y4,z4 = interpolateToGrid(XSCALE,YSCALE,x2,y2,z2)
    return x4,y4,z4

def convertImage(filename):
    #filename = os.path.join(skimage.data_dir, filename)
    img = io.imread(filename)
    newImg = None
    if filename.endswith(".png"):
        newImg = skimage.color.rgb2gray(skimage.color.rgba2rgb(img))
    else:
        newImg = skimage.color.rgb2gray(img)
    newImg = skimage.transform.rescale(newImg, (IMG_RES/len(newImg),IMG_RES/len(newImg[0])), anti_aliasing=True)
    x = []
    y = []
    z = []
    for i in range(len(newImg)):
        for j in range(len(newImg[i])):
            x.append(i)
            y.append(j)
            z.append(newImg[i][j])
    return x,y,z

def interpolateToGrid(xScale,yScale,x,y,z):
    xGrid = []
    yGrid = []
    for i in range(xScale):
        for j in range(yScale):
            xGrid.append(i)
            yGrid.append(j)
    points = np.array([x,y]).T
    values = np.array(z)
    return xGrid,yGrid,interp.griddata(points,values,(xGrid,yGrid),method='cubic')

def positivify(x):
    newX = x.copy()
    min = np.min(newX)
    for i in range(len(x)):
        newX[i] = newX[i] - min
    return newX

def scaleTo(x, newScale):
    newX = x.copy()
    max = np.max(newX)
    for i in range(len(x)):
        newX[i] = newX[i] * (newScale/max)
    return newX

def graphProcess(x, y, z, xScale, yScale, zScale):
    return scaleTo(positivify(x), xScale), scaleTo(positivify(y), yScale), scaleTo(positivify(z), zScale)