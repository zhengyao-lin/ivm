import std
import image
import io

img = image.bmp.parse("test_img.bmp")
print(img.width())
print(img.height())

// print(img.encode("bmp", "wow.bmp"))

ret

// -> "num: 200"
// -> "num: 150"
