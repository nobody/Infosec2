echo "Encrypting 25 chars"
./enc test1 tmp.25 > tmp.25.e
echo "Decrypting 25 chars"
./dec test1 tmp.25.e > tmp.25.o
diff tmp.25 tmp.25.o -a

echo
echo "Encrypting 22 chars"
./enc test1 tmp.22 > tmp.22.e
echo "Decrypting 22 chars"
./dec test1 tmp.22.e > tmp.22.o
diff tmp.22 tmp.22.o -a

echo
echo "Encrypting 20 chars"
./enc test1 tmp.20 > tmp.20.e
echo "Decrypting 20 chars"
./dec test1 tmp.20.e > tmp.20.o
diff tmp.20 tmp.20.o -a


FILE=MSG1
echo
echo "Encrypting $FILE"
./enc test1 "$FILE"> "$FILE.e"
echo "Decrypting $FILE"
./dec test1 "$FILE.e" > "$FILE.o"
diff "$FILE" "$FILE.o" -a

FILE=MSG2
echo
echo "Encrypting $FILE"
./enc test1 "$FILE"> "$FILE.e"
echo "Decrypting $FILE"
./dec test1 "$FILE.e" > "$FILE.o"
diff "$FILE" "$FILE.o" -a

FILE=MSG3
echo
echo "Encrypting $FILE"
./enc test1 "$FILE"> "$FILE.e"
echo "Decrypting $FILE"
./dec test1 "$FILE.e" > "$FILE.o"
diff "$FILE" "$FILE.o" -a
