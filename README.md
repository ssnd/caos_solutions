# Решения задач по курсу "Архитектура Компьютеров и Операционных Систем" -- 3 и 4 семестры.
Здесь архив моих решений по этому курсу.
В папке `solutions` все решения. Лучший кодстайл и оптимальные решения не гарантируются. 
В папке `parser` есть код парсера который использовался чтобы в конце семестра стянуть все задачи с ejudge.  

*Дисклеймер*: списывать нехорошо и задачи решать нужно самостоятельно, потому что курс полезный и нужный.

# Установка парсера
- Заходим в директорию со скриптом `cd parser`
- `python3 -m venv .env`
- `source ./env/bin/activate`
- `pip install -r requirements.txt`


# Использование
```bash
Usage: ejudge_parser.py [OPTIONS]

Options:
  --count INTEGER       Number of problems the script should download
                        (starting with id=1).
  --username TEXT       This script uses your username/pass to access the
                        ejudge interface.
  --password TEXT       This script uses your username/pass to access the
                        ejudge interface.
  --contest_id TEXT     Contest id to use (you can find it in your ejudge
                        login interface url with the url key `contest_id`)
  --problem_id INTEGER  Problem id to download, (--count ignored if true)
  --save_folder TEXT    path to save to problems to (default `./content`)
  --ejudge_url TEXT     ejudge url (default `https://ejudge.atp-fivt.org`)
  --help                Show this message and exit.
```