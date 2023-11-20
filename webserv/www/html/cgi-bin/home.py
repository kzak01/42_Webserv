#!/usr/bin/python3

#a = b
#while (1) : pass

from classes import *

DB = Database()
HD = Header()
ENV = Environment()

if ENV.get_field("REQUEST_METHOD") == "POST":
	bd = ENV.get_field("BODY")
	if bd == None:
		POST = Post("")
	else:
		POST = Post(ENV.get_field("BODY"))
else:
	POST = Post("")

def put_registration(tried:bool=False) -> str:
		return f"""<h2 class="subtitle" style="margin-left: 5px; margin-right: 5px"> Sign In(UP) and Out as you do with your home</h2>
		
		<form method="POST" action="/cgi-bin/home.py">
			  	<input name="login" value="0" style="display:none">
			  	<center><input type="submit"  value="Sign In webpage"></center>
		</form>
		<form method="POST" action="/cgi-bin/home.py">
			<div class="form_container">
			  <p> Username.. <input type="text" name="username"> </p>
			  <p> Password... <input type="password" name="psw"> </p>
			  <input name="register" value="0" style="display:none">
			  <input type="submit" value="Submit" style="display:none">
			  {"<p> failed, username already exists </p>" if tried else ""}
			  <p class="pmini">© 42webserver</p>
			</div>
		  </form>"""

def	put_login(tried:bool=False) -> None:
		return (f"""<h2 class="subtitle"> Log in </h2>
		  <form method="POST" action="/cgi-bin/home.py">
			  	<input name="register" value="0" style="display:none">
			  	<center><input type="submit"  value="Sign Up webpage"></center>
		   </form>
		<form method="POST" action="/cgi-bin/home.py">
			<div class="form_container">
			  <p> Username.. <input type="text" name="username"> </p>
			  <p> Password... <input type="password" name="psw"> </p>
			  <input name="login" value="0" style="display:none">
			  <input type="submit" value="Submit" style="display:none">
			  {"<p> Wrong credentials, try again...</p>" if tried else ""}
			  <p class="pmini">© 42webserver</p>
			</div>
		  </form>""")

def put_home(username:str="defaultuser") -> str:
	return (f"""<h2 class="subtitle"> Your Web Home </h2>
		<center><p> You have succesfully entered your cobweb </p></center>
		<center><p> Welcome back {username}! </p></center>""")

user = POST.get_field("username")
psw = POST.get_field("psw")
reg = POST.get_field("register")
log = POST.get_field("login")

webpage:str = ""

if (reg == None and log == None) or (reg and user == None and psw == None):
	webpage = get_homepage(put_registration(), HD)
elif reg:
	if user and psw:
		if not DB.user_exist(user):
			DB.insert_user(user, psw)
			webpage = get_homepage(put_login(), HD)
		else:
			webpage = get_homepage(put_registration(True), HD)
	else:
		webpage = get_homepage(put_registration(True), HD)
elif log:
	if user and psw:
		if DB.check_user(user, psw):
			webpage = get_homepage(put_home(user), HD, user)
		else:
			webpage = get_homepage(put_login(True), HD)
	elif user or psw:
		webpage = get_homepage(put_login(True), HD)
	else:
		webpage = get_homepage(put_login(), HD)

print(webpage)
