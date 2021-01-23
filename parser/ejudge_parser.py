from operator import attrgetter
from typing import Union

import bs4
import requests
import re
from collections import namedtuple
import logging

logger = logging.getLogger("ejudge_parser")
logging.basicConfig(level=logging.DEBUG)

Solution = namedtuple("Solution", "id, name, status, source")


class Parser:
    def __init__(self):
        self.session: requests.Session = requests.Session()

    def login(self):
        # Fill in your details here to be posted to the login form.
        payload = {
            'contest_id': '3',
            'role': '0',
            'prob_name': '',
            'login': '',
            'password': '',
            "locale_id": 0,
            "action_2": "Log in",
        }
        p = self.session.post('https://ejudge.atp-fivt.org/client', data=payload)
        soup: bs4.BeautifulSoup = bs4.BeautifulSoup(p.content)

        links: bs4.ResultSet = soup.find_all("a", href=True)

        assert len(links) > 0

        link: bs4.Tag = links[0]

        link_text: str = link.attrs['href']

        sid_list: list = re.findall("SID=(.*)&", link_text)

        assert len(sid_list) == 1
        self.sid: str = sid_list[0]

    @staticmethod
    def find_td_source(row: bs4.ResultSet) -> Solution:
        parsed_row = row.findAll("td")
        id: int = int(parsed_row[0].text)
        name: str = parsed_row[3].text
        status: str = parsed_row[5].text
        source: str = parsed_row[8].find('a').attrs['href']

        return Solution(id, name, status, source)

    def convert_table_to_list(self, rows: bs4.ResultSet) -> list:
        solution_list: list = []

        for row in rows:
            solution: Solution = self.find_td_source(row)
            solution_list.append(solution)

        solution_list = sorted(solution_list, key=attrgetter('id'), reverse=True)

        return solution_list

    def sort_by_status(self, problem_list: list, status: str = "OK"):
        filtered_list = list(filter(lambda x: x.status == status, problem_list))
        return filtered_list

    def get_problem_name(self, problem_container):
        problem_name_container: Union[bs4.BeautifulSoup, bs4.NavigableString] = problem_container.find("h3")
        problem_name: str = problem_name_container.text.split("Problem ")[1]
        return problem_name

    def find_code(self, soup):
        c = soup.find("pre", class_="line-numbers")
        code = c.find('code')
        return code.text

    def download_solution(self, solution, path):
        r = self.session.get(solution.source)
        soup = bs4.BeautifulSoup(r.content)
        code = self.find_code(soup)
        with open(f"{path}/{solution.name}.txt", "w") as f:
            f.write(code)

        logger.info(f"problem {solution.name} saved")

    def parse_problem(self, problem_id: int) -> None:
        url = f"https://ejudge.atp-fivt.org/client?SID={self.sid}&action=139&prob_id={problem_id}"
        r = self.session.get(url)
        soup: bs4.BeautifulSoup = bs4.BeautifulSoup(r.content)

        problem_container: Union[bs4.BeautifulSoup, bs4.NavigableString] = soup.find("table", class_="probNav")
        table: Union[bs4.Tag, bs4.NavigableString, int] = problem_container.find("table", class_="table")
        if table is None:
            logger.error(f"parsing error at id = {problem_id}")
            return
        table_rows: bs4.ResultSet = table.findAll("tr")
        problem_list = self.convert_table_to_list(table_rows[1:])
        if len(problem_list) == 0:
            logger.error("solution list empty")
            return

        filtered_problems = self.sort_by_status(problem_list)
        if len(filtered_problems) == 0:
            logger.error("didn't find any correct solutions")
            return

        solution: Solution = filtered_problems[0]
        self.download_solution(solution, f"./content/")

    def run(self):
        self.login()
        for i in range(0, 50):
            self.parse_problem(i)


if __name__ == "__main__":
    Parser().run()
