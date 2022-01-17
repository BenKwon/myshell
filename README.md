# myshell
> Shell written by C
## How to run
```bash
./makefile
```
## supported command<br>
- foreground and background execution (&) ex) (sleep 4;ls) & <br>
- multiple commands separated by semicolons <br>
- command group (ex. (ls -l | grep ^d; pwd ;date) > out.txt <br>
- history command and excute history command(history, !history_num) <br>
- shell redirection (>, >>, >|, <) <br>
- shell pipe (ls –la | more) <br>
- Multiple pipe (ls | grep “^d” | more) <br>
- cd command <br>
