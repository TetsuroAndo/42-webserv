#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import cgitb
import json

# add parent dir and cgi-bin dir to import storage/auth
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(__file__)), 'cgi-bin'))
import auth

cgitb.enable()

print("Content-Type: application/json; charset=utf-8\n\n")

user = auth.get_authenticated_user()

print(json.dumps({"user": user}, ensure_ascii=False))
