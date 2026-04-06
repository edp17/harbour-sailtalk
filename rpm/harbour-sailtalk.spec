Name:       harbour-sailtalk
Summary:    Native Sailfish OS voice calling app
Version:    0.1.0
Release:    4
License:    BSD-3-Clause
URL:        https://github.com/edp17/harbour-sailtalk
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  cmake
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-webrtc-1.0)
BuildRequires:  pkgconfig(gstreamer-sdp-1.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(Qt5WebSockets)
BuildRequires:  pkgconfig(sailfishapp)

Requires:       gstreamer1.0
Requires:       gstreamer1.0-plugins-bad
Requires:       gstreamer1.0-plugins-good
Requires:       libsailfishapp-launcher

%description
SailTalk is a native Sailfish OS app for direct internet audio calling,
with a WebRTC-based media engine and simple signaling.

%prep
%autosetup

%build
%cmake
%cmake_build

%install
%cmake_install

%files
/usr/bin/harbour-sailtalk
/usr/share/harbour-sailtalk
/usr/share/applications/harbour-sailtalk.desktop
/usr/share/icons/hicolor/172x172/apps/harbour-sailtalk.png
