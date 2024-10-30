# CodinCame Summer Challenge 2024 Olymbits CLI runner

This is a referee for CodinGame's Summer Challenge 2024 with command line interface adapted for usage with [cg-brutaltester](https://github.com/dreignier/cg-brutaltester)

https://www.codingame.com/contests/summer-challenge-2024-olymbits

## Build

- install Java 17 and Maven
- run in command line `mvn package` in the root dir where pom.xml file is

The compiled jar file is in ./target/summer-2024-olympics-1.0-SNAPSHOT.jar

## Run

Build cg-brutaltester as described in its readme-file ([cg-brutaltester](https://github.com/dreignier/cg-brutaltester))

To copy both referee and cg-brutaltester JAR files, i.e. summer-2024-olympics-1.0-SNAPSHOT.jar and cg-brutaltester-1.0.0-SNAPSHOT.jar to same directory make handling paths easier.

This is a sample command that works, you can change optptions as described in the BrutalTester readme-file.

```
java -jar cg-brutaltester-1.0.0-SNAPSHOT.jar \
    -r "java --add-opens java.base/java.lang=ALL-UNNAMED -jar -Dleague.level=3 summer-2024-olympics-1.0-SNAPSHOT.jar" \
    -p1 "python3 player1.py" \
    -p2 "python3 player2.py" \
    -p3 "python3 player3.py" \
    -s     \
    -t 4   \
    -n 10
```

Option `-Dleague.level=3` above defines League-3 that is valid for Bronze and higher leagues.

---
