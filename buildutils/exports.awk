{
        if ((($2 == "T") || ($2 == "D") || ($2 == "B")) \
                && ( substr($1,1,1) != ".")) {
            if (substr ($1, 1, 7) != "__sinit" &&
                    substr ($1, 1, 7) != "__sterm") {
                if (substr ($1, 1, 5) == "__tf1")
                    print (substr ($1, 7))
                else if (substr ($1, 1, 5) == "__tf9")
                    print (substr ($1, 15))
                else
                    print $1
            }
        }
    }

