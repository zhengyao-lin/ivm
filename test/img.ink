import std
import image
import io

img = image.bmp.parse("test_img.bmp")
print(img.width())
print(img.height())

loc pixel = fn pix: {
	r: pix & 0xff,
	g: (pix >>> 8) & 0xff,
	b: (pix >>> 16) & 0xff,

	to_s: fn "(#{base.r}, #{base.g}, #{base.b})",
	raw: fn base.r | ((base.g & 0xff) << 8) | ((base.b & 0xff) << 16)
}

for loc i in range(img.width()):
	for loc j in range(img.height()): {
		// print(pix(img.get(i, j)))
		pix = pixel(img.get(i, j))
		img.set(i, j, ~pix.raw())
	}

img.encode("bmp", "test_img2.bmp")

ret

// -> "num: 200"
// -> "num: 150"
