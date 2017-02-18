import std

name = "rod"
job = "cleaner"

print("this is #{name}, I'm a #{job}")

print("#{job}")

print("job: #{job}")

print("#{\"job: \" + job}")

print("\#{job}")
print("#\{job}")
print("#{{job}}")

print("#{{job}}, yeah")

// print("#{{job}}")

print("#{\"#{job}\"}")
print("#{\"\\#{job}\"}")

print("#{\"#{\\\"#{job}\\\"}\"}")

ret

// -> "str: this is rod, I'm a cleaner"
// -> "str: cleaner"
// -> "str: job: cleaner"
// -> "str: job: cleaner"
// -> "str: #{job}"
// -> "str: #{job}"
// -> "str: cleaner"
// -> "str: cleaner, yeah"
// -> "str: cleaner"
// -> "str: #{job}"
// -> "str: cleaner"
