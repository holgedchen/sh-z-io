﻿//Copyright (c) 2007, 2008 John Leitch
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
//john.leitch5@gmail.com
using System;
using System.Collections.Generic;
using System.Text;

namespace TFTPClient
{
    public struct TransferOptions
    {
        private TransferType _action;

        public TransferType Action
        {
            get { return _action; }
            set { _action = value; }
        }

        private string _localFilename;

        public string LocalFilename
        {
            get { return _localFilename; }
            set { _localFilename = value; }
        }

        private string _remoteFilename;

        public string RemoteFilename
        {
            get { return _remoteFilename; }
            set { _remoteFilename = value; }
        }

        private string _host;

        public string Host
        {
            get { return _host; }
            set { _host = value; }
        }
    }
}
