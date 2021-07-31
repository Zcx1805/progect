from selenium import webdriver
import time
import unittest

class imageTest(unittest.TestCase):
    def setUp(self) :
        self.driver=webdriver.Chrome()
        self.driver.maximize_window()
        self.url="http://192.168.107.128:9898/"

    def tearDown(self):
        self.driver,quit()

    def test_insert_order(self):
        driver=self.driver
        driver.get(self.url)
        driver.implicitly_wait(10)

        driver.find_element_by_xpath('//*[@id="tables"]/div/div/table/tbody/tr[1]/td[3]/div/label/input').click()
        time.sleep(3)
        driver.find_element_by_xpath('//*[@id="tables"]/div/div/table/tfoot/tr/th/button').click()
        time.sleep(3)
        alert=driver.switch_to.alert
        alert.accept()
        time.sleep(3)

    if __name__=="__main__":
        unittest.main(verbosity=0)

