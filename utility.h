//
// Copyright 2010, 2011 Tom Klein.
//
// This file is part of cstitch.
//
// cstitch is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef UTILITY_H
#define UTILITY_H

#include <algorithm>

#include <QtCore/QAbstractEventDispatcher>
#include <QtGui/QApplication>
#include <QtGui/QAction>
#include <QtGui/QProgressDialog>
#include <QtGui/QCloseEvent>

// special floss color codes
const int DMC_COUNT = 427; // number of dmc colors
// DMC doesn't assign codes for white or ecru
const int WHITE_CODE = -10;
const int ECRU_CODE = -11;

// a "bool with three states"
enum triStateValue {triFalse = 0, triTrue = 1, triNoop = 2};
// using a struct instead of a bare enum so that triState can be forward
// declared
class triState {
 public:
  triState(triStateValue value) : value_(value) {}
  explicit triState(bool value) {
    value_ = value ? triTrue : triFalse;
  }
  bool toBool() const {
    return (value_ == triTrue) ? true : false;
  }
  bool operator==(const triStateValue value) const {
    return value_ == value;
  }
  bool operator!=(const triStateValue value) const {
    return value_ != value;
  }
 private:
  triStateValue value_;
};

inline void clearEventQueue() {

  QAbstractEventDispatcher* dispatcher =
    QAbstractEventDispatcher::instance();
  if (dispatcher) {
    if (dispatcher->hasPendingEvents()) {
      dispatcher->processEvents(QEventLoop::AllEvents);
    }
  }
}

// int to qstring
inline QString itoqs(int i) {
  return QString("%1").arg(i);
}
// real to qstring
inline QString rtoqs(qreal r) {
  return QString("%1").arg(r, 0, 'f', 2);
}

// width of a string in the application font ("string width")
inline int sWidth(const QString& s) {
  static QFontMetrics qfm(QApplication::font());
  return qfm.boundingRect(s).width();
}

// return true if the application font contains the character <c>
inline bool charInFont(const QChar& c) {
  static QFontMetrics qfm(QApplication::font());
  return qfm.inFont(c);
}

// width of the string <n> in the application font
inline int sWidth(const int& n) {
  return sWidth(itoqs(n));
}

// height of the string <s> in the application font
inline int sHeight(const QString& s) {
  static QFontMetrics qfm(QApplication::font());
  return qfm.boundingRect(s).height();
}

// height of the string <n> in the application font
inline int sHeight(const int& n) {
  return sHeight(itoqs(n));
}

inline void setFontHeight(QFont* font, int height) {

  for (int j = 1; j < 100; ++j) {
    font->setPointSize(j);
    const QFontMetrics metrics(*font);
    if (metrics.height() > height - 1) {
      if (j > 1) {
        font->setPointSize(j-1);
      }
      else {
        font->setPointSize(1); // !
      }
      break;
    }
  }
}

// return the smallest application font with height at least <height>
inline QFont fontForHeight(int height) {

    static QFont font = QApplication::font();
    setFontHeight(&font, height);
    return font;
}

// return the average width of a character in <font> (rounded up)
// [by character we mean ascii characters 32-127]
int averageCharacterWidth(const QFont& font);

// return the part of <s> before the last '.'
inline QString base(const QString& s) {
  return s.left(s.lastIndexOf('.'));
}

// return the part of <s> after the last '.'
inline QString extension(const QString& s) {
  return s.section('.', -1);
}

// read the file <fileName> into a single string and return it (or the
// empty string if the file can't be opened)
// WARNING: don't use this on "big" files!
QString readTextFile(const QString& fileName);

// functor for finding an action (with data a type with
// a name method) with a given <name>
template<class T>
class findActionName {
 public:
  explicit findActionName(const QString& name) : name_(name) {}
  bool operator()(const QAction* action) const {
    return !action->data().isNull() &&
      action->data().canConvert<T>() &&
      action->data().value<T>()->name() == name_;
  }
 private:
  QString name_;
};

// search the list of actions <list> and return the first for which
// <findFunctor> returns true, else return NULL
template<class T>
QAction* actionFromImageName(const QList<QAction*>& list,
                             const T& findFunctor) {

  QList<QAction*>::const_iterator it =
    std::find_if(list.begin(), list.end(), findFunctor);
  if (it != list.end()) {
    return *it;
  }
  else {
    return NULL;
  }
}

// popup a file dialog for the user to choose a file name and return the
// file selected (empty string if the file dialog was cancelled); if
// <displayWarning> then warn the user that all work will be lost if they
// continue and give them the opportunity to cancel this dialog, in which
// case the empty string is returned
QString getNewImageFileName(QWidget* activeWindow, bool displayWarning);

// groupProgressDialog is used during project restore in combination with
// altMeters to display which image is currently being restored and the
// current progress on that image.  The way it works:
// when restore is started, altMeter is provided with a static
// groupProgressDialog which causes all altMeters constructed during
// restore to pass their data to the groupProgressDialog instead of a
// normal progressMeter.  Each time an altMeter is constructed the
// groupDialog gets reset so it can show progress for the new meter.
// The project opener must keep track of the current image count via
// bumpCount.  The combination of the bumpCounts and the resets tracks
// individual image progress during the project open. When altMeter's
// static groupDialog isn't set the altMeter behaves as a normal
// progress dialog (so users of the altMeter never know the difference).
class groupProgressDialog : public QProgressDialog {
 public:
  explicit groupProgressDialog(int numDialogs);
  void reset(int min, int max) {
    setRange(min, max);
    setLabelText("Recreating image " + QString::number(count_) +
                         "/" + QString::number(numDialogs_));
  }
  // ignore show requests from group members
  void show() { return; }
  void showGroup() { QProgressDialog::show(); }
  void bumpCount() { ++count_; }
  void closeEvent(QCloseEvent* event) {
    // currently cancelling project opening midstream will almost
    // certainly result in a segfault
    event->ignore();
  }

 private:
  int count_;
  const int numDialogs_;
};

// see documentation above for groupProgressDialog on the interaction
// between that class and this.
// setGroupMeter MUST NOT be called while any altMeter is active
class altMeter {
 public:
  altMeter(const QString& labelText, const QString& cancelButtonText,
           int minimum, int maximum);
  ~altMeter() {
    if (!groupDialog_) {
      delete dialog_;
    }
  }
  void setMinimumDuration(int min) { dialog_->setMinimumDuration(min); }
  bool wasCanceled() const { return dialog_->wasCanceled(); }
  void setValue(int value) {
    dialog_->setValue(value);
  }
  void show() {
    dialog_->show();
  }
  // MUST NOT be called while any altMeter is active
  static void setGroupMeter(groupProgressDialog* meter) {
    groupDialog_ = meter;
  }

 private:
  QProgressDialog* dialog_;
  static groupProgressDialog* groupDialog_;
};

#endif