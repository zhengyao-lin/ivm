import std
import image
import io

img = image.bmp.parse("test_img.bmp")
print(img.width())
print(img.height())

loc pixel = fn pix: {
	a: pix & 0xff,
	b: (pix >>> 8) & 0xff,
	g: (pix >>> 16) & 0xff,
	r: (pix >>> 24) & 0xff,

	to_s: fn "(#{base.r}, #{base.g}, #{base.b})",
	raw: fn (base.a & 0xff) | ((base.b & 0xff) << 8) | ((base.g & 0xff) << 16) | ((base.r & 0xff) << 24)
}

// get: 960380928
// set: 3334586367

// get: 960380928
// set: 3334586367

// 960380928

// 960380928, -960380929
// 960380928, 3334586367
for loc i in range(img.width()):
	for loc j in range(img.height()): {
		// print(pix(img.get(i, j)))
		pix = pixel(img.get(i, j))
		// print(img.get(i, j) + ", " + ~img.get(i, j))
		img.set(i, j, ~img.get(i, j))
	}

img.encode("bmp", "test_img2.bmp")

image.image(100, 100).encode("bmp", "test_img3.bmp")

// print(image.bmp.parse("t1.bmp").width())

ret

// -> "num: 200"
// -> "num: 150"
