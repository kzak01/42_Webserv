#!/usr/bin/python3

import os, sys, getopt, json

homepath = "./www/html/homepy.html"
# homepath = "../homepy.html"

class	Header():
	content_type = "text/html"

	def __str__(self, body:str=""):
		return f"""HTTP/1.1 200 OK\r\nContent-type: {self.content_type}; charset=utf-8\r\nContent-Lenght: {len(body)}\r\nConnection: close\r\n\r\n"""

class	Database():
	path:str = "./www/dbs/users.json"
	db:dict = {}

	def __init__(self):
		# self.path = "./../../dbs/users.json"
		self.load()

	def load(self):
		with open(self.path, "r") as f:
			self.db = json.load(f)

	def save(self):
		with open(self.path, "w") as f:
			json.dump(self.db, f, indent=4)

	def user_exist(self, username:str) -> bool:
		return (True if username in self.db else False)

	def check_user(self, username:str, psw:str) -> bool:
		if username in self.db:
			if self.db[username] == psw:
				return True
		return False

	def insert_user(self, username:str, psw:str) -> None:
		self.db[username] = psw
		self.save()

class	Environment():

	def check_field(self, field:str) -> bool:
		return True if field in os.environ else False

	def get_field(self, field:str) -> str:
		return (os.getenv(field) if self.check_field(field) else None)

class	Post():
	variables:dict = {}

	def __init__(self, body:str):
		if not len(body) > 0: return 
		strings = body.split("&")
		for string in strings:
			text = string.split("=")
			self.variables[text[0]] = text[1]

	def check_field(self, field:str) -> bool:
		return True if field in self.variables else False

	def get_field(self, field:str) -> str:
		return (self.variables[field] if self.check_field(field) else None)

def get_homepage(filler:str="", header:Header=None, filler2:str=None):
	temp_body = ""
	with open(homepath, "r") as f:
		lines = f.readlines()

	to_replace = "<!-- <div>to_replace</div> -->"
	to_replace2 = "Who are thou? @_@"
	for line in lines:
		if to_replace in line:
			line = line.replace(to_replace, filler)
		if filler2:
			if to_replace2 in line:
				line = line.replace(to_replace2, f"Hello {filler2}! :)")
		temp_body += line

	body = header.__str__(temp_body)
	body += temp_body

	return body
