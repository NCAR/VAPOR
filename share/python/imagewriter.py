# outfile   : string
# width     : image width
# height    : image height
# rgbbuffer : buffer of R, G, B values

def drawpng( outfile, width, height, rgbbuffer ):
    import matplotlib
    matplotlib.use('AGG')
    import matplotlib.image as mpimg
    import numpy as np

    buf = np.array( rgbbuffer, dtype=np.uint8 )
    buf = buf.reshape( height, width, 3 )
    mpimg.imsave( outfile, buf, format='png' )

    return 0


def main():
    width =64 
    height = 128
    buf = []
    for y in range( height ):
        for x in range( width ):
            buf.append(x)
            buf.append(y)
            buf.append(0)

    drawpng( "rgb.png", width, height, buf )


if __name__ == "__main__":
    main()

