1. **Running the Code:**
   1. ensure that you are in the ventilation_control folder
   2. uncomment your test and comment out other unnecessary ones
   3. press new terminal
   4. if you are using the esp32-s3 type 'idf.py set-target esp32s3' otherwise skip this step
   5. (in that terminal) type 'idf.py build'
   6. resolve all compiler issues
   7. make sure to connect esp32
   8. type 'idf.py flash'
   9. type 'idf.py monitor'
   10. ctrl + ] if you need to stop monitoring or execution
2. **How to push code to github:**
a.  'git checkout -b <your-new-branch-name>'
b. 'git add .'
c. git commit -m "<some message>"
d. git remote -v (make sure that you are committing to the correct github repo)
e. git push --set-upstream origin <your-new-branch-name>
f. git push

7. 
