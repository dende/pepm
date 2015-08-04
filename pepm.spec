Summary: Password manager with GTK2 GUI and Privacy Enhancement
Name: pepm
Version: 0.79
Release: 1%{?dist}
License: GPLv2+
Group: Applications/Productivity
Source: http://als.regnet.cz/%{name}/download/%{name}-%{version}.tar.bz2
URL: https://github.com/dende/pepm
BuildRequires: gtk2-devel, libxml2-devel, desktop-file-utils, gettext
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

%description
Figaro's Password Manager 2 is a program that allows you to securely store the
passwords using GTK2 interface. Features include:
- Passwords are encrypted with the AES-256 algorithm.
- Copy passwords or usernames to the clipboard/primary selection.
- If the password is for a web site, FPM2 can keep track of the URLs of your
  login screens and can automatically launch your browser. In this capacity,
  FPM2 acts as a kind of bookmark manager.
- Combine all three features: you can configure FPM2 to bring you to a web
  login screen, copy your username to the clipboard and your password to the
  primary selection, all with a single button click.
- FPM2 also has a password generator that can choose passwords for you. It
  allows you to determine how long the password should be, and what types of
  characters (lower case, upper case, numbers and symbols) should be used.
  You can even have it avoid ambiguous characters such as a capital O or the
  number zero.
- Auto-minimise and/or auto-locking passwords database after configurable time
  to the tray icon.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%find_lang %{name}

desktop-file-install \
  --vendor fedora \
  --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
  %{buildroot}%{_datadir}/applications/%{name}.desktop

%clean
rm -rf %{buildroot}

%files -f %{name}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog NEWS README TODO
%{_bindir}/pepm
%{_datadir}/pixmaps/pepm
%{_mandir}/man1/pepm.1.gz
%{_datadir}/applications/*.desktop

%changelog
* Mon Jan 17 2011 Aleš Koval <als@regnet.cz> - 0.79-1
- Update to 0.79
- Fixed crash due to incorrectly call xmlCleanupParser() (#669102)

* Wed Mar  3 2010 Aleš Koval <als@regnet.cz> - 0.78-1
- Update to 0.78
- Fixed crash in master password dialog after open keyfile (#569520)

* Tue Feb 23 2010 Aleš Koval <als@regnet.cz> - 0.77-1
- Update to 0.77
- Fix ImplicitDSOLinking (#565058)

* Mon Nov  9 2009 Aleš Koval <als@regnet.cz> - 0.76.1-1
- Update to 0.76.1

* Fri Nov  6 2009 Aleš Koval <als@regnet.cz> - 0.76-1
- Update to 0.76

* Tue Apr  7 2009 Christoph J. Thompson <cjsthompson@gmail.com>
- Pixmaps are now in /usr/share/pixmaps/fpm2

* Mon Mar  2 2009 Aleš Koval <als@regnet.cz> - 0.75-1
- Update to 0.75

* Mon Jul  7 2008 Aleš Koval <als@regnet.cz> - 0.72-1
- Update to 0.72

* Thu May  8 2008 Aleš Koval <als@regnet.cz> - 0.71-2
- Fix %%files section (#444830)
- desktop-file-install now delete original .desktop file

* Wed Apr 30 2008 Aleš Koval <als@regnet.cz> - 0.71-1
- Initial build.
