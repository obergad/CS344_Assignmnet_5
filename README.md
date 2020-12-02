# CS344_Assignmnet_5
Assignment 5 for Oregon State University's CS344




enc_server 125696 &    
enc_client plaintext1 key70000 125696 > ciphertext

dec_server 156412 &
dec_client ciphertext key70000 156412



175471
16013

gcc -std=gnu99 -o server server.c
gcc -std=gnu99 -o client client.c




"enc \n" = encryption
"\n end \n" = end of message
