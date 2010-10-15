default: enc dec forge

enc: encrypt.cpp
	g++ -g $< -o $@

dec: decrypt.cpp
	g++ -g $< -o $@

forge: forge.cpp
	g++ -g $< -o transform
