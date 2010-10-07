default: enc dec

enc: encrypt.cpp
	g++ -g $< -o $@

dec: decrypt.cpp
	g++ -g $< -o $@
