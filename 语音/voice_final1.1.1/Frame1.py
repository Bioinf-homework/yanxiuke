#Boa:Frame:Frame1

import wx
import wx.lib.analogclock

import os

import random

def create(parent):
    return Frame1(parent)

[wxID_FRAME1, wxID_FRAME1ANALOGCLOCK1, wxID_FRAME1B1, wxID_FRAME1B2, 
 wxID_FRAME1B3, wxID_FRAME1PANEL1, wxID_FRAME1STATICTEXT1, 
] = [wx.NewId() for _init_ctrls in range(7)]

class Frame1(wx.Frame):
    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_FRAME1, name='', parent=prnt,
              pos=wx.Point(673, 448), size=wx.Size(404, 140),
              style=wx.DEFAULT_FRAME_STYLE | wx.CAPTION | wx.CLOSE_BOX | wx.FRAME_EX_CONTEXTHELP | wx.SIMPLE_BORDER,
              title=u'\u77ed\u8bcd\u5339\u914d')
        self.SetClientSize(wx.Size(388, 102))
        self.SetBackgroundColour(wx.Colour(129, 154, 214))

        self.panel1 = wx.Panel(id=wxID_FRAME1PANEL1, name='panel1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(388, 102),
              style=wx.TAB_TRAVERSAL)
        self.panel1.SetBackgroundColour(wx.Colour(213, 234, 232))
        self.panel1.Bind(wx.EVT_LEFT_DOWN, self.OnPanel1LeftDown)

        self.b1 = wx.Button(id=wxID_FRAME1B1, label=u'\u5f55\u97f3', name=u'b1',
              parent=self.panel1, pos=wx.Point(28, 54), size=wx.Size(75, 24),
              style=0)
        self.b1.SetForegroundColour(wx.Colour(57, 176, 45))
        self.b1.Bind(wx.EVT_BUTTON, self.OnB1Button, id=wxID_FRAME1B1)

        self.b2 = wx.Button(id=wxID_FRAME1B2, label=u'\u8bad\u7ec3', name=u'b2',
              parent=self.panel1, pos=wx.Point(120, 54), size=wx.Size(75, 24),
              style=0)
        self.b2.SetForegroundColour(wx.Colour(146, 156, 14))
        self.b2.Bind(wx.EVT_BUTTON, self.OnB2Button, id=wxID_FRAME1B2)

        self.b3 = wx.Button(id=wxID_FRAME1B3, label=u'\u5339\u914d', name=u'b3',
              parent=self.panel1, pos=wx.Point(212, 54), size=wx.Size(75, 24),
              style=0)
        self.b3.SetForegroundColour(wx.Colour(216, 48, 90))
        self.b3.Bind(wx.EVT_BUTTON, self.OnB3Button, id=wxID_FRAME1B3)

        self.staticText1 = wx.StaticText(id=wxID_FRAME1STATICTEXT1,
              label=u'\u7b80\u964b\u4f46\u7528\u5fc3\u7684demo',
              name='staticText1', parent=self.panel1, pos=wx.Point(72, 16),
              size=wx.Size(204, 25), style=0)
        self.staticText1.SetFont(wx.Font(18, wx.SWISS, wx.NORMAL, wx.NORMAL,
              False, u'\u65b9\u6b63\u8212\u4f53'))
        self.staticText1.SetForegroundColour(wx.Colour(22, 156, 190))
        self.staticText1.Bind(wx.EVT_LEFT_DOWN, self.OnStaticText1LeftDown)
        self.staticText1.Bind(wx.EVT_KEY_DOWN, self.OnStaticText1KeyDown)

        self.analogClock1 = wx.lib.analogclock.analogclock.AnalogClock(id=wxID_FRAME1ANALOGCLOCK1,
              name='analogClock1', parent=self.panel1, pos=wx.Point(296, 24),
              size=wx.Size(80, 52), style=0)

    def __init__(self, parent):
        self._init_ctrls(parent)

    def OnStaticText1LeftDown(self, event):
        #print "You have click the statictext"
        r = random.randint(0,255)
        g = random.randint(0,255)
        b = random.randint(0,255)
        #print r,g,b
        self.staticText1.SetForegroundColour(wx.Colour(r, g, b))
        self.staticText1.SetLabel(u'\u7b80\u964b\u4f46\u7528\u5fc3\u7684demo')
        event.Skip()

    def OnB1Button(self, event):
        print "luyin"
        os.startfile("py_voice1.exe")
        event.Skip()

    def OnB2Button(self, event):
        print "MFCC"
        os.startfile("MFCC.exe")
    
        event.Skip()

    def OnB3Button(self, event):
        print "pipei"
        os.startfile("match.exe")
        event.Skip()

    def OnStaticText1KeyDown(self, event):
        print 'key'
        event.Skip()

    def OnPanel1LeftDown(self, event):
        r = random.randint(0,255)
        g = random.randint(0,255)
        b = random.randint(0,255)
        print r,g,b
        self.panel1.SetBackgroundColour(wx.Colour(r, g, b))
        event.Skip()
        
