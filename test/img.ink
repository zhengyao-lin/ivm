import std
import image
import io

img = image.bmp.parse("test_img1.bmp")
print(img.width())
print(img.height())

loc pixel = fn pix: {
	a: pix & 0xff,
	b: (pix >>> 8) & 0xff,
	g: (pix >>> 16) & 0xff,
	r: (pix >>> 24) & 0xff,

	to_s: fn "(#{base.r}, #{base.g}, #{base.b}, #{base.a})",
	raw: fn (base.a & 0xff) | ((base.b & 0xff) << 8) | ((base.g & 0xff) << 16) | ((base.r & 0xff) << 24)
}

for loc i in range(img.width()):
	for loc j in range(img.height()): {
		// print(pix(img.get(i, j)))
		pix = pixel(img.get(i, j))
		// print(pixel(~img.get(i, j)))
		// print(img.get(i, j) + ", " + ~img.get(i, j))
		img.set(i, j, ~img.get(i, j))
	}

img.encode("bmp", "test_img2.bmp")

img = image.bmp.parse("test_img3.bmp")
print(img.width())
print(img.height())

for loc i in range(img.width()):
	for loc j in range(img.height()): {
		pix = pixel(img.get(i, j))
		img.set(i, j, ~img.get(i, j))
	}

img.encode("bmp", "test_img4.bmp")

image.image(100, 100).encode("bmp", "test_img5.bmp")

// print(image.bmp.parse("t1.bmp").width())

ret

// -> "num: 200"
// -> "num: 150"
// -> "num: 128"
// -> "num: 128"
