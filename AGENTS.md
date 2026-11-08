# Project context

Work only on Simulated RF Transceiver Driver in C in this folder. This is one of three separate projects, not a monorepo.
The user requested software only, a separate GitHub repository per project under sukh-j-14, and Vercel deployment if deployable. They want to continue each project in its own app Project chat.
Keep the native C/C++ core. A browser companion can be deployed to Vercel but must not be presented as native FreeRTOS or actual RF hardware execution.
Use Neon Postgres plus Prisma for relational data; MongoDB plus Mongoose for document data if needed. Never commit credentials.
Read README.md and existing code before changing anything. Start by running build.ps1 and reviewing the tests. Existing native tests passed on 2026-11-08. Publication, demos, and storage remain pending. Do not invent resume results or claim the user has mastered these concepts.
