//! w2db: persistência do derust. Postgres atrás de um serviço gRPC num Unix socket, para o DBSrv.
//! Ver docs/FASE_2_5.md (B2) e docs/W2DB.md.

pub mod admin;
pub mod auth;
pub mod check;
pub mod discord;
pub mod legacy;
pub mod metrics;
pub mod service;
pub mod store;
pub mod transfer;

pub mod pb {
    tonic::include_proto!("w2db.v1");
}
