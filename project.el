;;; Project ---  BlockTracker Project file for Emacs users.
;;; Commentary:
;;;    Eval this file!

;;; Code:

(require 'projectile)

(puthash (projectile-project-root)
         ;; "ninja && ninja -t compdb compile >! compile_commands.json"
         "ninja"
         projectile-compilation-cmd-map)

(puthash (projectile-project-root)
         "./test/bin/btest"
         projectile-test-cmd-map)

(global-set-key (kbd "<f5>") (lambda()
                               (interactive)
                               (cd (projectile-project-root))
                               (shell-command "python bootstrap.py --cxx=clang")
                               ))

(global-set-key (kbd "C-<f5>") (lambda()
                                 (interactive)
                                 (cd (projectile-project-root))
                                 (shell-command "python bootstrap.py --debug --cxx=clang")
                                 ))

(global-set-key (kbd "<f6>") #'projectile-compile-project)
(global-set-key (kbd "<f7>") #'projectile-test-project)

(provide 'project)
;;; project.el ends here
