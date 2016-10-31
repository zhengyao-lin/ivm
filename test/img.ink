import image
import io

fp = io.file("test/test_img.bmp")
buf = fp.readBuffer()

img = image.bmp.parse(buf)
print(img.width())
print(img.height())

ret

// -> "num: 200"
// -> "num: 150"
