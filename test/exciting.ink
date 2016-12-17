import std
import ulist

list.proto.need = fn count: {
	if base.size() < count:
		raise exception("too small stack")
}

loc binop = fn stack, op: {
	stack.need(2)
	stack.push(op(stack.pop(), stack.pop()))
}

a = {
	stack: [],
	val: 0,
	exec: [
		fn stack: { // push 1
			stack.push(1)
		},

		binop(_, fn a, b: a + b),
		binop(_, fn a, b: a - b),
		binop(_, fn a, b: a * b),
		binop(_, fn a, b: (a / b).round()),

		fn stack: {
			print([ c.char() for loc c in stack ].reduce(fn a, b: a + b, ""))
		},

		fn stack: {
			print("top: " + stack.top())
		},

		fn stack: {
			stack.need(1)
			stack.push(stack.top())
		},

		fn stack: { // 9
			loc sum = 0

			while stack.size():
				sum = sum + stack.pop()

			stack.push(sum)
		}
	],
	-@: fn: {
		// print(base.val)
		loc val = base.val
		base.val = 0

		if val == 0: ret

		if val <= base.exec.size():
			base.exec[val - 1](base.stack)
		else:
			print("unexpected exciting code " + val)

		base
	},

	!: fn: {
		base.val = base.val + 1
		base
	},

	|: fn input: {
		base.stack = base.stack + input.ords()
	}
}

// str: [ 101, 120, 99, 105, 116, 105, 110, 103, 33 ]

// a | "hi"

   -!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!a



        -!-!-!-!-!-!-!-!-!-!            -!-!-!-!-!-!-!-!-!-!a
        -!                -!            -!                -!a
        -! -!             -!            -! -!             -!a
        -!                -!            -!                -!a
-!      -!-!-!-!-!-!-!-!-!              -!-!-!-!-!-!-!-!-!         -!a
-!                                                                -!a
 -!                                                              -!a
  -!                                                            -!a
    -!                                                        -!a
      -!                                                    -!a
        -!                                               -!a
           -!                                         -!a
               -!                                  -!a
                   -! -! -! -! -! -! -! -! -! -!a




                     -!!!!!
                      !!!!a
                     -!!!-!
                     -!!!!-
                     !!!!-!
                     !-!-!-
                     !!-!!-
!-!-!-!!-!-!a;-!!!a;-!!!!!!!!a;-!!-!!-!!-!-!-!-!!
!!-!!!!-!!!!-!!-!-!-!!-!-!-!!-!-!-!!-!-!a;-!!a;-!
!!!!!!!a;-!!-!-!!-!!-!!-!-!-!!-!-!-!!!!-!!!!-!!!!
                     -!!-!-
                     !-!!-!
                     -!-!!-
                     !-!-!!
                     -!-!a;
                     -!!!a;
                     -!!!!!
                     !!!a;-
                     !!-!!-



                     !!-!-!
                   -!!-!-!-
                  !!-!-!a;-
                 !!a;-!!!!!
               !!!a;-!!-!-!
              !-!!-!!-!!-!!
             -!-!-!!-!-!-!!
           -!-!-!!   -!-!-!
          !-!-!a;-   !!a;-!
                     !!!!!!
                     !a;-!!
                     -!-!!-
                     !!-!!-
                     !!-!!-
                     !-!-!!
                     -!-!-!
                     !-!-!-
                     !!-!-!
                     -!!-!-
                     !a;-!!
                     !a;-!!
                     !!!!!!
                     a;-!!-
          !!-!-!!-!-!-!!-!-!a;-!!a;-!!!
          !!!!!a;-!!-!!-!!-!-!!-!-!-!!-
          !-!-!!-!-!a;-!!!a;-!!-!-!!!!-




   !!!!-!!!!-!!!!-!!-!-!-!!-!-!-!!-!-!-!!-!-!-!!-!-!-!!!!!!a;ret

// -> "str: exciting"
