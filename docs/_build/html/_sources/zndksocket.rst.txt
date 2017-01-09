==========
zndksocket
==========

zndksocket

.. seqdiag::

   seqdiag {
     activation = none;

     client; server;

     server  ->  server [label="socket"                      ];
     server  ->  server [label="bind"                        ];
     server  ->  server [label="listen"                      ];
     client  ->  client [label="socket"                      ];

     client  ->> server [label="connect", rightnote="accept" ];

     client  ->> server [label="send"   , rightnote="receive"];
     client <<-  server [label="send"   ,  leftnote="receive"];
   }
