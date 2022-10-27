import sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.interpolate as interp
import skimage.io as io
import skimage

imageExts = ["png","jpg","jpeg"]

IMG_RES = 100
COMPARE_RES = 30000
SHIFT_RES = 10
XSCALE = 3
YSCALE = 3
ZSCALE = 3

INTERP_STYLE = 'cubic'

diffs = []


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

def allPointsIn(x,y,z,xMin,yMin,xMax,yMax):
    retX = []
    retY = []
    retZ = []
    for i in range(len(x)):
        if x[i] >= xMin and x[i] < xMax and y[i] >= yMin and y[i] < yMax:
            retX.append(x[i])
            retY.append(y[i])
            retZ.append(z[i])
    return retZ

def segmentMeans(xOffset,yOffset,xScale,yScale,x,y,z):
    retX = []
    retY = []
    retZ = []
    xLim = xOffset
    yLim = yOffset
    for i in range(xScale):
        yLim = yOffset
        for j in range(yScale):
            retX.append(xLim)
            retY.append(yLim)
            points = allPointsIn(x,y,z,xLim,yLim,xLim+1,yLim+1)
            if len(points) > 0:
                retZ.append((sum(points)/len(points)))
                #print("xLim: " + str(xLim) + " yLim: " + str(yLim) + " points: " + str(points) + " mean: " + str(sum(points)/len(points)))
            else:
                retZ.append(0)
                #print("xLim: " + str(xLim) + " yLim: " + str(yLim) + " points: " + str(points) + " mean: " + str(0))
            yLim+=1
        xLim+=1
    return retX,retY,retZ

def interpolateToGrid(xOffset,yOffset,xScale,yScale,x,y,z):
    xGrid = []
    yGrid = []
    for i in range(xScale):
        for j in range(yScale):
            xGrid.append(i+xOffset)
            yGrid.append(j+yOffset)
    points = np.array([x,y]).T
    values = np.array(z)
    return xGrid,yGrid,interp.griddata(points,values,(xGrid,yGrid),method=INTERP_STYLE)

def compareGraphs(x,y,z,x2,y2,z2,xMin,xMax,yMin,yMax):
    rng = np.random.default_rng()
    xCoords = (xMax-xMin)*rng.random(COMPARE_RES)+xMin
    yCoords = (yMax-yMin)*rng.random(COMPARE_RES)+yMin
    points = np.array([x,y]).T
    values = np.array(z)
    points2 = np.array([x2,y2]).T
    values2 = np.array(z2)
    zCoords = interp.griddata(points,values,(xCoords,yCoords),method=INTERP_STYLE)
    zCoords2 = interp.griddata(points2,values2,(xCoords,yCoords),method=INTERP_STYLE)
    diff = zCoords2-zCoords
    diff = diff[~np.isnan(diff)]
    diff = diff**2
    return np.mean(diff)

def findMinFromCoords(x,y,z,filename,display=False,save=False,summary=False):
    x2,y2,z2 = graphProcess(x,y,z, XSCALE, YSCALE, ZSCALE)
    if display:
        fig1 = plt.figure()
        ax = None
        ax2 = None
        ax3 = None
        ax4 = None
        if summary:
            ax = fig1.add_subplot(221,projection='3d',title="Scaled Data")
            ax2 = fig1.add_subplot(222,projection='3d',title="Minimized Error")
            ax3 = fig1.add_subplot(223,projection='3d',title="0 Offset Data")
            ax4 = fig1.add_subplot(224,projection='3d',title="Maximized Error")
        else:
            ax = fig1.add_subplot(211,projection='3d',title="Scaled Data")
            ax2 = fig1.add_subplot(212,projection='3d',title="Minimized Error")
        ax.scatter3D(x2, y2, z2, c=z2)
    minR = 5
    maxR = 0
    RZero = -1
    minROffsets = []
    maxROffsets = []
    minZ = []
    minY = []
    minX = []
    maxZ = []
    maxY = []
    maxX = []
    zeroZ = []
    zeroY = []
    zeroX = []
    offsets = np.linspace(0,1,SHIFT_RES)
    for xOffset in offsets:
        for yOffset in offsets:
            x4,y4,z4 = interpolateToGrid(xOffset,yOffset,XSCALE,YSCALE,x2,y2,z2)
            r4 = compareGraphs(x2,y2,z2,x4,y4,z4,0,20,0,20)
            print("xOffset: " + str(xOffset) + " yOffset: " + str(yOffset) + " r4: " + str(r4))
            if r4 < minR:
                minR = r4
                minROffsets = [xOffset,yOffset]
                minX = x4
                minY = y4
                minZ = z4
            if r4 > maxR:
                maxR = r4
                maxROffsets = [xOffset,yOffset]
                maxX = x4
                maxY = y4
                maxZ = z4
            if np.where(offsets==xOffset)[0] == 0 and np.where(offsets==yOffset)[0] == 0:
                RZero = r4
                zeroX = x4
                zeroY = y4
                zeroZ = z4
    print("minR: " + str(minR) + " minROffsets: " + str(minROffsets))
    print("maxR: " + str(maxR) + " maxROffsets: " + str(maxROffsets))
    diffs.append((minR,RZero,maxR))
    if display:
        ax2.scatter3D(minX, minY, minZ, c=minZ)
        if summary:
            ax3.scatter3D(zeroX, zeroY, zeroZ, c=zeroZ)
            ax4.scatter3D(maxX, maxY, maxZ, c=maxZ)

    if save:
        f=open(filename.split(".")[0] + "_min.xyz","w")
        for i in range(len(minX)):
            f.write(str(minX[i]-minROffsets[0]) + " " + str(minY[i]-minROffsets[1]) + " " + str(minZ[i]) + "\n")
        f.close()
    if display:
        plt.show()
    return minX,minY,minZ

def findMin(filename,image=False,display=False,save=False,gray=False,summary=False):
    x,y,z = [],[],[]
    if image:
        x,y,z = convertImage(filename,gray)
    else:
        x,y,z = np.genfromtxt(filename, delimiter=" ", unpack=True, autostrip=True)
    return findMinFromCoords(x,y,z,filename,display,save,summary)

def convertImage(filename,gray=False):
    #filename = os.path.join(skimage.data_dir, filename)
    img = io.imread(filename)
    newImg = None
    if gray:
        if len(img[0][0])==4:
            newImg = skimage.color.rgb2gray(skimage.color.rgba2rgb(img))
        elif len(img[0][0])==3:
            newImg = skimage.color.rgb2gray(img)
    else:
        if len(img[0][0])==4:
            newImg = 650 - ((250 / 270) * skimage.color.rgb2hsv(skimage.color.rgba2rgb(img))[:,:,0])
        elif len(img[0][0])==3:
            newImg = 650 - ((250 / 270) * skimage.color.rgb2hsv(img)[:,:,0])
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

def outputImage(x,y,z,filename):
    f = open(filename, "w")
    for i in range(len(x)):
        f.write(str(x[i]) + " " + str(y[i]) + " " + str(z[i]) + "\n")
    f.close()

def displayDiffs(t):
    fig = plt.figure()
    ax = fig.add_subplot(111,title=t)
    x = []
    y = []
    colors = []
    for i,d in enumerate(diffs):
        x.append(i)
        x.append(i)
        x.append(i)
        y.append(d[0])
        y.append(d[1])
        y.append(d[2])
        colors.append((0,0,1))
        colors.append((0,1,0))
        colors.append((1,0,0))
    ax.scatter(x,y,c=colors)
    plt.show()


"""
-d display graphs
-s save the best representation as ###_min.xyz
-g use grayscale instead of hue/wavelength processing
-a when displaying, show a graph of best, worst, and default approximations along with raw data
"""
if __name__ == "__main__":
    diffs = []
    if len(sys.argv) > 1:
        findMin(sys.argv[1],sys.argv[1].split(".")[-1].lower() in imageExts,"-d" in sys.argv,"-s" in sys.argv,"-g" in sys.argv,"-a" in sys.argv)
    else:
        for i in range(3,5):
            findMin("testImage"+str(i+1)+".png",True,True,False,True,False)
    displayDiffs("Best (Blue), Worst (Red), and Default (Green) Approximations")
